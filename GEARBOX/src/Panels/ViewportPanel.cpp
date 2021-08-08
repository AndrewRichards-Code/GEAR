#include "ViewportPanel.h"
#include "directx12/D3D12Image.h"
#include "vulkan/VKImage.h"

#include "backends/imgui_impl_vulkan_with_textures.h"

using namespace gear;
using namespace graphics;

using namespace miru::crossplatform;

using namespace gearbox;
using namespace panels;

uint32_t ViewportPanel::s_ViewportPanelCount = 0;

ViewportPanel::ViewportPanel()
{
	s_ViewportPanelCount++;
	m_ViewID = s_ViewportPanelCount;
}

ViewportPanel::~ViewportPanel()
{
	s_ViewportPanelCount--;
}

void ViewportPanel::Draw(const Ref<RenderSurface>& renderSurface, const Ref<imgui::UIContext>& context)
{
	if (ImGui::Begin("Viewport"))
	{
		bool resized = false;
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((size.x != m_CurrentSize.x) || (size.y != m_CurrentSize.y))
		{
			renderSurface->GetContext()->DeviceWaitIdle();

			//Ensure RenderSurface size is never { 0, 0 }.
			m_CurrentSize.x = std::max(size.x, 1.0f);
			m_CurrentSize.y = std::max(size.y, 1.0f);
			renderSurface->Resize((uint32_t)m_CurrentSize.x, (uint32_t)m_CurrentSize.y);
			resized = true;
		}

		Ref<miru::crossplatform::Framebuffer> framebuffer = renderSurface->GetHDRFramebuffers()[0];
		Ref<ImageView> colourImageView = framebuffer->GetCreateInfo().attachments[0];
		uint32_t width = colourImageView->GetCreateInfo().pImage->GetCreateInfo().width;
		uint32_t height = colourImageView->GetCreateInfo().pImage->GetCreateInfo().height;
		
		if (context->GetCreateInfo().window->GetGraphicsAPI() == GraphicsAPI::API::D3D12)
		{
			if (!m_ImageID || resized)
			{
				Ref<miru::d3d12::Image> d3d12ColourImage = ref_cast<miru::d3d12::Image>(colourImageView->GetCreateInfo().pImage);

				ID3D12Device* device = (ID3D12Device*)renderSurface->GetDevice();
				UINT handleIncrement = (device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * m_ViewID;
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
				cpuHandle.ptr = context->m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart().ptr + handleIncrement;
				device->CreateShaderResourceView(d3d12ColourImage->m_Image, nullptr, cpuHandle);

				m_ImageID = (ImTextureID)(context->m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart().ptr + handleIncrement);
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
				VkDevice device = *(VkDevice*)renderSurface->GetDevice();
				if (!m_VKSampler)
					vkCreateSampler(device, &sCI, nullptr, &m_VKSampler);


				if (m_ImageID && resized)
				{
					vkFreeDescriptorSets(device, context->m_VulkanDescriptorPool, 1, (VkDescriptorSet*)&m_ImageID);
				}

				m_ImageID = ImGui_ImplVulkan_AddTexture(m_VKSampler, vkColourImageView->m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			
		}
		ImGui::Image(m_ImageID, ImVec2(width, height));
	}
	ImGui::End();
}
