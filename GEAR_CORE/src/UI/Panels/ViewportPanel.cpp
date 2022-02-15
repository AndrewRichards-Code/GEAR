#include "gear_core_common.h"
#include "ViewportPanel.h"
#include "Panels.h"
#include "UI/ComponentUI/ComponentUI.h"

#include "Graphics/Renderer.h"

using namespace gear;
using namespace ui;
using namespace panels;
using namespace objects;

using namespace miru::crossplatform;

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
			m_CI.renderer->ResizeRenderPipelineViewports((uint32_t)m_CurrentSize.x, (uint32_t)m_CurrentSize.y);
			
			resized = true;
		}

		const uint32_t& m_FrameIndex = m_CI.renderer->GetFrameIndex();
		Ref<Framebuffer> framebuffer = m_CI.renderer->GetRenderSurface()->GetHDRFramebuffers()[m_FrameIndex];
		Ref<ImageView> colourImageView = framebuffer->GetCreateInfo().attachments[0];
		uint32_t width = colourImageView->GetCreateInfo().pImage->GetCreateInfo().width;
		uint32_t height = colourImageView->GetCreateInfo().pImage->GetCreateInfo().height;
		
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
		
	}
	ImGui::End();
}

void ViewportPanel::UpdateCameraTransform()
{
	Ref<Camera> camera = m_CI.renderer->GetCamera();
	if (!camera)
		return;

	//View matrix update
	camera->m_CI.perspectiveParams.aspectRatio = m_CI.renderer->GetRenderSurface()->GetRatio();
	
	//Mouse Control
	if (ImGui::IsWindowFocused())
	{
		//Stop camera snapping back when new input is detected
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		{
			m_InitialMousePosition = GetMousePositionInViewport();
		}

		//Main update
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
		{
			const float2& mouse = GetMousePositionInViewport();
			float2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			m_Yaw += delta.x;
			m_Pitch += delta.y;

			camera->m_CI.transform.orientation = Quaternion::FromEulerAngles(float3(m_Pitch, -m_Yaw, m_Roll));
		}
		if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Middle))
		{
			const float2& mouse = GetMousePositionInViewport();
			float2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			float x_offset = delta.x;
			float y_offset = delta.y;

			camera->m_CI.transform.translation +=
				(camera->m_Right * -x_offset) +
				(camera->m_Up * y_offset);
		}
	}

	camera->Update();
}

float2 ViewportPanel::GetMousePositionInViewport()
{
	double mousePosition_x, mousePosition_y;
	m_CI.renderer->GetWindow()->GetMousePosition(mousePosition_x, mousePosition_y);

	return float2((float)mousePosition_x, (float)mousePosition_y);
}
