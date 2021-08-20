#include "gearbox_common.h"
#include "ViewportPanel.h"

#include "directx12/D3D12Image.h"
#include "vulkan/VKImage.h"

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
	if (ImGui::Begin("Viewport"))
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
		
		if (GraphicsAPI::IsD3D12())
		{
			if (!m_ImageID || resized)
			{
				Ref<miru::d3d12::Image> d3d12ColourImage = ref_cast<miru::d3d12::Image>(colourImageView->GetCreateInfo().pImage);

				ID3D12Device* device = (ID3D12Device*)m_CI.renderer->GetDevice();
				UINT handleIncrement = (device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * m_ViewID;
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
				cpuHandle.ptr = m_CI.uiContext->m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart().ptr + handleIncrement;
				device->CreateShaderResourceView(d3d12ColourImage->m_Image, nullptr, cpuHandle);

				m_ImageID = (ImTextureID)(m_CI.uiContext->m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart().ptr + handleIncrement);
			}
		}
		else
		{
			if (!m_ImageID || resized)
			{
				Ref<miru::vulkan::ImageView> vkColourImageView = ref_cast<miru::vulkan::ImageView>(colourImageView);

				VkSamplerCreateInfo sCI;
				sCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				sCI.pNext = nullptr;
				sCI.flags = 0;
				sCI.magFilter = VK_FILTER_LINEAR;
				sCI.minFilter = VK_FILTER_LINEAR;
				sCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				sCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sCI.mipLodBias = 0.0f;
				sCI.anisotropyEnable = false;
				sCI.maxAnisotropy = 1.0f;
				sCI.compareEnable = false;
				sCI.compareOp = VK_COMPARE_OP_NEVER;
				sCI.minLod = 0;
				sCI.maxLod = 0;
				sCI.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
				sCI.unnormalizedCoordinates = false;
				VkDevice device = *(VkDevice*)m_CI.renderer->GetDevice();
				if (!m_VKSampler)
					vkCreateSampler(device, &sCI, nullptr, &m_VKSampler);


				if (m_ImageID && resized)
				{
					vkFreeDescriptorSets(device, m_CI.uiContext->m_VulkanDescriptorPool, 1, (VkDescriptorSet*)&m_ImageID);
				}

				m_ImageID = ImGui_ImplVulkan_AddTexture(m_VKSampler, vkColourImageView->m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
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
