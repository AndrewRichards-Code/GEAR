#include "gearbox_common.h"
#include "ViewportPanel.h"
#include "ComponentUI/ComponentUI.h"

using namespace gear;
using namespace graphics;

using namespace miru::crossplatform;

using namespace gearbox;
using namespace panels;

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
	if (ImGui::Begin("Viewport", &m_Open))
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
		Ref<miru::crossplatform::Framebuffer> framebuffer = m_CI.renderer->GetRenderSurface()->GetHDRFramebuffers()[m_FrameIndex];
		Ref<ImageView> colourImageView = framebuffer->GetCreateInfo().attachments[0];
		uint32_t width = colourImageView->GetCreateInfo().pImage->GetCreateInfo().width;
		uint32_t height = colourImageView->GetCreateInfo().pImage->GetCreateInfo().height;
		
		m_ImageID = componentui::GetTextureID(colourImageView, m_CI.uiContext, resized, m_ImageID);
		ImGui::Image(m_ImageID, ImVec2(static_cast<float>(width), static_cast<float>(height)));

		
	}
	ImGui::End();
}

void ViewportPanel::UpdateCameraTransform()
{
	Ref<objects::Camera> camera = m_CI.renderer->GetCamera();
	if (!camera || !ImGui::IsWindowFocused())
		return;

	//Stop camera snapping back when new input is detected
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
	{
		m_InitialMousePosition = GetMousePositionInViewport();
	}
	
	//Main update
	if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
	{
		const Vec2& mouse = GetMousePositionInViewport();
		Vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		m_Yaw += delta.x;
		m_Pitch += delta.y;

		camera->m_CI.transform.orientation = Quat::FromEulerAngles(Vec3(m_Pitch, -m_Yaw, m_Roll));
	}
	if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Middle))
	{
		const Vec2& mouse = GetMousePositionInViewport();
		Vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		float x_offset = delta.x;
		float y_offset = delta.y;

		camera->m_CI.transform.translation +=
			(camera->m_Right * -x_offset) +
			(camera->m_Up * y_offset);
	}
	camera->Update();
}

Vec2 ViewportPanel::GetMousePositionInViewport()
{
	double mousePosition_x, mousePosition_y;
	m_CI.renderer->GetWindow()->GetMousePosition(mousePosition_x, mousePosition_y);

	return std::move(Vec2((float)mousePosition_x, (float)mousePosition_y));
}
