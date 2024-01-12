#include "gear_core_common.h"
#include "UI/Panels/ViewportPanel.h"
#include "UI/Panels/SceneHierarchyPanel.h"
#include "UI/Panels/RendererPropertiesPanel.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Picker.h"
#include "Graphics/Window.h"

using namespace gear;
using namespace ui;
using namespace panels;
using namespace objects;

using namespace miru::base;

using namespace mars;

uint32_t ViewportPanel::s_ViewportPanelCount = 0;

ViewportPanel::ViewportPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::VIEWPORT;
	m_CI = *pCreateInfo;

	s_ViewportPanelCount++;
	m_ViewID = s_ViewportPanelCount;
}

ViewportPanel::~ViewportPanel()
{
	s_ViewportPanelCount--;
}

void ViewportPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Viewport", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		UpdateCameraTransform();

		ImGuiMouseCursor cursorType = ImGui::GetMouseCursor();
		bool mouseLeftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		if (cursorType != ImGuiMouseCursor_Arrow && mouseLeftDown)
		{
			ImGui::End();
			return;
		}

		bool resized = false;
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((size.x != m_CurrentSize.x) || (size.y != m_CurrentSize.y))
		{
			m_CI.renderer->GetContext()->DeviceWaitIdle();

			//Ensure RenderSurface size is never { 0, 0 }.
			m_CurrentSize.x = std::max(size.x, 1.0f);
			m_CurrentSize.y = std::max(size.y, 1.0f);

			m_CI.renderer->GetRenderSurface()->Resize((uint32_t)m_CurrentSize.x, (uint32_t)m_CurrentSize.y);
			
			resized = true;
		}

		const uint32_t& m_FrameIndex = m_CI.renderer->GetFrameIndex();
		ImageViewRef colourImageView = m_CI.renderer->GetRenderSurface()->GetColourSRGBImageView();
		uint32_t width = colourImageView->GetCreateInfo().image->GetCreateInfo().width;
		uint32_t height = colourImageView->GetCreateInfo().image->GetCreateInfo().height;
		
		m_ImageID = componentui::GetTextureID(colourImageView, UIContext::GetUIContext(), resized);
		ImGui::Image(m_ImageID, ImVec2(static_cast<float>(width), static_cast<float>(height)));

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".gsf");
			if (payload)
			{
				std::string filepath = (char*)payload->Data;
				if (std::filesystem::exists(filepath))
				{
					Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
					if (sceneHierarchyPanel)
					{
						sceneHierarchyPanel->GetScene()->LoadFromFile(filepath, UIContext::GetUIContext()->GetWindow());
						sceneHierarchyPanel->UpdateWindowTitle();
					}
				}
			}
		}

		if (ImGui::IsWindowFocused())
		{
			Ref<RendererPropertiesPanel> rendererPropertiesPanel = UIContext::GetUIContext()->GetEditorPanelsByType<RendererPropertiesPanel>()[0];
			if (rendererPropertiesPanel)
			{
				for (Ref<ViewportPanel> viewport : UIContext::GetUIContext()->GetEditorPanelsByType<ViewportPanel>())
				{
					if (viewport.get() == this)
						rendererPropertiesPanel->SetViewportPanel(viewport);
				}
			}
		}
		
	}
	ImGui::End();
}

void ViewportPanel::UpdateCameraTransform()
{
	Ref<Camera> camera = m_CI.renderer->GetCamera();
	if (!camera)
		return;

	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (!sceneHierarchyPanel)
		return;
	Transform transform = Transform();
	bool found = false;
	const auto& vCameraComponents = sceneHierarchyPanel->GetScene()->GetRegistry().view<scene::TransformComponent, scene::CameraComponent>();
	for (auto& entity : vCameraComponents)
	{
		Ref<Camera>& _camera = vCameraComponents.get<scene::CameraComponent>(entity);
		found = _camera == camera;
		if (found)
		{
			transform = vCameraComponents.get<scene::TransformComponent>(entity);
			break;
		}
	}
	if (!found)
		return;

	//Aspect Ratio update
	const float& aspectRatio = m_CI.renderer->GetRenderSurface()->GetRatio();
	if (camera->m_CI.projectionType == Camera::ProjectionType::ORTHOGRAPHIC)
	{
		//camera->m_CI.orthographicParams.left = -aspectRatio;
		//camera->m_CI.orthographicParams.right = +aspectRatio;
	}
	else if (camera->m_CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
	{
		camera->m_CI.perspectiveParams.aspectRatio = aspectRatio;
	}
	else
	{
		GEAR_FATAL(ErrorCode::OBJECTS | ErrorCode::INVALID_VALUE, "Unknown projection type.");
	}

	//View matrix update
	
	//Mouse Control
	if (ImGui::IsWindowFocused())
	{
		
		const float& multiplier = ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift) ? 4.0f : 1.0f;
		const float& speed = 1.0f * multiplier;

		//Stop camera snapping back when new input is detected
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		{
			m_InitialMousePosition = GetMousePositionInViewport();
		}

		//Main update UE5 control style.
		
		//Pan
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
		{
			const float2& mouse = GetMousePositionInViewport();
			float2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			m_Yaw += delta.x;
			m_Pitch += delta.y;

			transform.orientation = Quaternion::FromEulerAngles(float3(m_Pitch, m_Yaw, m_Roll));
		}
		
		//Translate: Left, right, up and down.
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Middle))
		{
			const float2& mouse = GetMousePositionInViewport();
			float2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			float x_offset = delta.x;
			float y_offset = delta.y;

			transform.translation += ((camera->m_Right * -x_offset) + (camera->m_Up * y_offset)) * speed;
		}

		//Translate: Back and forth.
		const float& newMouseScrollWheel = GetMouseScrollWheel();
		if (newMouseScrollWheel != m_InitalMouseScrollWheel)
		{
			transform.translation += (camera->m_Direction * -(newMouseScrollWheel - m_InitalMouseScrollWheel));
			m_InitalMouseScrollWheel = newMouseScrollWheel;
		}

		//Picker objects in viewport.
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			float2 mouse = GetMousePositionInViewport();
			Ref<Model> nearestModel = graphics::Picker::GetNearestModel(m_CI.renderer->GetModelQueue(), camera, mouse, m_CurrentSize);
			if (nearestModel)
			{
				const auto& vModelComponents = sceneHierarchyPanel->GetScene()->GetRegistry().view<scene::TransformComponent, scene::ModelComponent>();
				for (auto& entity : vModelComponents)
				{
					Ref<Model> _model = vModelComponents.get<scene::ModelComponent>(entity);
					if (_model == nearestModel)
					{
						scene::Entity _entity;
						_entity.m_CI = { sceneHierarchyPanel->GetScene().get() };
						_entity.m_Entity = entity;

						sceneHierarchyPanel->GetSelectedEntity() = _entity;
					}
				}
			}
		}
	}

	camera->Update(transform);

	for (auto& entity : vCameraComponents)
	{
		Ref<Camera>& _camera = vCameraComponents.get<scene::CameraComponent>(entity);
		found = _camera == camera;
		if (found)
		{
			vCameraComponents.get<scene::TransformComponent>(entity) = transform;
			break;
		}
	}
}

float2 ViewportPanel::GetMousePositionInViewport()
{
	double mousePosition_x, mousePosition_y;
	m_CI.renderer->GetWindow()->GetMousePosition(mousePosition_x, mousePosition_y);
	float2 mousePosition(static_cast<float>(mousePosition_x), static_cast<float>(mousePosition_y)); //Mouse position in Window space.

	//https://github.com/ocornut/imgui/issues/2486
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	float2 mainViewportPosition(mainViewport->WorkPos.x, mainViewport->WorkPos.y); //Main Viewport position in OS Screen space.

	const ImVec2& _regionMin = ImGui::GetWindowContentRegionMin();
	float2 regionMin(_regionMin.x, _regionMin.y); //Content region minimum position in Window space.

	const ImVec2& _windowPos = ImGui::GetWindowPos(); 
	float2 windowPos(_windowPos.x, _windowPos.y); //Window position in OS Screen space.

	float2 mouse = mousePosition - (regionMin + windowPos) + mainViewportPosition;

	return float2(std::clamp(mouse.x, 0.0f, m_CurrentSize.x), std::clamp(mouse.y, 0.0f, m_CurrentSize.y));
}

float ViewportPanel::GetMouseScrollWheel()
{
	double mouseScroll;
	m_CI.renderer->GetWindow()->GetScrollPosition(mouseScroll);

	return static_cast<float>(mouseScroll);
}
