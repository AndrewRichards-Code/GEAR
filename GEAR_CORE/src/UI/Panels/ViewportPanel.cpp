#include "gear_core_common.h"
#include "UI/Panels/ViewportPanel.h"
#include "UI/Panels/SceneHierarchyPanel.h"
#include "UI/Panels/RendererPropertiesPanel.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

#include "Graphics/DebugRender.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Picker.h"
#include "Graphics/Window.h"

#include "ImGuizmo/ImGuizmo.h"

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
		m_CI.renderer->SetDebugRendering(m_DebugRendering);

		//Don't display view during resize.
		ImGuiMouseCursor cursorType = ImGui::GetMouseCursor();
		bool mouseLeftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		if (cursorType != ImGuiMouseCursor_Arrow && mouseLeftDown)
		{
			ImGui::End();
			return;
		}

		//Check for resize.
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

		//Draw main view.
		const uint32_t& m_FrameIndex = m_CI.renderer->GetFrameIndex();
		ImageViewRef colourImageView = m_CI.renderer->GetRenderSurface()->GetColourSRGBImageView();
		uint32_t width = colourImageView->GetCreateInfo().image->GetCreateInfo().width;
		uint32_t height = colourImageView->GetCreateInfo().image->GetCreateInfo().height;
		
		m_ImageID = componentui::GetTextureID(colourImageView, UIContext::GetUIContext(), resized);
		ImGui::Image(m_ImageID, ImVec2(static_cast<float>(width), static_cast<float>(height)));

		DrawGuizmos();

		//Load scene via drag and drop
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

		//Set the RendererPropertiesPanel's viewport.
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

		DrawOverlay(id);

		UpdateCameraTransform();
		
	}
	ImGui::End();
}

void ViewportPanel::DrawOverlay(const std::string& parentPanelID)
{
	const Ref<Camera>& camera = m_CI.renderer->GetCamera();
	if (!camera)
		return;

	ImGui::SetCursorPos({ 5.0f + ImGui::GetWindowContentRegionMin().x, 5.0f + ImGui::GetWindowContentRegionMin().y });
	std::string id = parentPanelID + "_Overlay";

	ImGui::PushStyleColor(ImGuiCol_HeaderActive,	{ 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_Header,			{ 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered,	{ 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_ChildBg,			{ 0.0f, 0.0f, 0.0f, 0.2f });
	if (ImGui::BeginChild(id.c_str(), {250.0f, 150.0f}, true))
	{
		static bool open = false;
		if (ImGui::CollapsingHeader("Camera Setting", open ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			ui::componentui::DrawFloat("Camera Speed", m_CameraSpeed, 1.0f, 10.0f);
			ui::componentui::DrawCheckbox("Debug Rendering", m_DebugRendering);
		}
		if (ImGui::CollapsingHeader("Transform Settings", open ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			ui::componentui::DrawDropDownMenu("Type", m_TransformType);
			ui::componentui::DrawCheckbox("Snapping", m_TransformSnapping);
			ui::componentui::DrawFloat("Snapping Value", m_TransformSnappingValues[(size_t)m_TransformType], 0.0f);

		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor(4);
}

void ViewportPanel::DrawGuizmos()
{
	if (!m_DebugRendering)
		return;

	const Ref<Camera>& camera = m_CI.renderer->GetCamera();
	if (!camera)
		return;
	
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (!sceneHierarchyPanel)
		return;

	scene::Entity entity = sceneHierarchyPanel->GetSelectedEntity();
	if (entity && entity.HasComponent<scene::TransformComponent>())
	{
		ImGuizmo::SetOrthographic(camera->m_CI.projectionType == Camera::ProjectionType::ORTHOGRAPHIC);
		ImGuizmo::SetDrawlist();
		ImGuizmo::AllowAxisFlip(false);

		const ImVec2& windowPos = ImGui::GetWindowPos();
		ImGuizmo::SetRect(windowPos.x, windowPos.y, m_CurrentSize.x, m_CurrentSize.y);

		const Ref<graphics::Uniformbuffer<graphics::UniformBufferStructures::Camera>>& cameraUB = camera->GetCameraUB();

		const Camera::PerspectiveParameters& pp = camera->m_CI.perspectiveParams;
		float4x4 cameraProj = mars::float4x4::Perspective(pp.horizonalFOV, pp.aspectRatio, pp.zNear, pp.zFar);
		float4x4 cameraView = cameraUB->view;

		Transform& entityTransform = entity.GetComponent<scene::TransformComponent>().transform;
		float4x4 modelModl = TransformToMatrix4(entityTransform);

		cameraProj.Transpose();
		cameraView.Transpose();
		modelModl.Transpose();

		float snappingValues[3] = { m_TransformSnappingValues[(size_t)m_TransformType], m_TransformSnappingValues[(size_t)m_TransformType], m_TransformSnappingValues[(size_t)m_TransformType] };
		switch (m_TransformType)
		{
		default:
		case ViewportPanel::TransformationType::TRANSLATION:
		{
			ImGuizmo::Manipulate(cameraView.GetData(), cameraProj.GetData(), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, &(modelModl.a), nullptr, m_TransformSnapping ? snappingValues : nullptr);
			break;
		}
		case ViewportPanel::TransformationType::ROTATION:
		{
			ImGuizmo::Manipulate(cameraView.GetData(), cameraProj.GetData(), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, &(modelModl.a), nullptr, m_TransformSnapping ? snappingValues : nullptr);
			break;
		}
		case ViewportPanel::TransformationType::SCALE:
		{
			ImGuizmo::Manipulate(cameraView.GetData(), cameraProj.GetData(), ImGuizmo::OPERATION::SCALEU, ImGuizmo::MODE::LOCAL, &(modelModl.a), nullptr, m_TransformSnapping ? snappingValues : nullptr);
			break;
		}
		}

		m_GuizmoActive = ImGuizmo::IsUsing();
		if (m_GuizmoActive)
		{
			entityTransform = Matrix4ToTransform(modelModl.Transpose());
		}
	}

	std::vector<graphics::UniformBufferStructures::Model>& debugMatrices = graphics::DebugRender::GetDebugModelMatrices();
	debugMatrices.clear();

	const auto& vLightComponents = sceneHierarchyPanel->GetScene()->GetRegistry().view<scene::LightComponent, scene::TransformComponent>();
	for (const auto& entity : vLightComponents)
	{
		const Ref<Light>& light = vLightComponents.get<scene::LightComponent>(entity);
		const Transform& transform = vLightComponents.get<scene::TransformComponent>(entity);
		const Light::CreateInfo& CI = light->m_CI;

		graphics::UniformBufferStructures::Model model;
		model.modl = TransformToMatrix4(transform);
		model.texCoordScale0 = { CI.colour.r, CI.colour.g };
		model.texCoordScale1 = { CI.colour.b, CI.colour.a };
		debugMatrices.emplace_back(model);
	}
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
	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		
		const float& multiplier = ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift) ? 4.0f : 1.0f;
		const float& speed = m_CameraSpeed * multiplier;

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
			transform.translation += (camera->m_Direction * speed * -(newMouseScrollWheel - m_InitalMouseScrollWheel));
			m_InitalMouseScrollWheel = newMouseScrollWheel;
		}

		//Picker objects in viewport.
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left) && !m_GuizmoActive)
		{
			float2 mouse = GetMousePositionInViewport();

			Ref<Model> nearestModel = graphics::Picker::GetNearestModel(m_CI.renderer->GetModelQueue(), camera, mouse, m_CurrentSize);
			if (nearestModel)
			{
				const auto& vModelComponents = sceneHierarchyPanel->GetScene()->GetRegistry().view<scene::ModelComponent>();
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

			Ref<Light> nearestLight = graphics::Picker::GetNearestLight(m_CI.renderer->GetLights(), camera, mouse, m_CurrentSize);
			if (nearestLight)
			{
				const auto& vLightComponents = sceneHierarchyPanel->GetScene()->GetRegistry().view<scene::LightComponent>();
				for (auto& entity : vLightComponents)
				{
					Ref<Light> _light = vLightComponents.get<scene::LightComponent>(entity);
					if (_light == nearestLight)
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
	else
	{
		m_InitialMousePosition = GetMousePositionInViewport();
		m_InitalMouseScrollWheel = GetMouseScrollWheel();
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
