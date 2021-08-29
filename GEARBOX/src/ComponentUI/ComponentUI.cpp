#include "gearbox_common.h"
#include "NameComponentUI.h"

#include "directx12/D3D12Image.h"
#include "vulkan/VKImage.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

using namespace miru::crossplatform;

using namespace mars;

void gearbox::componentui::DrawStaticText(const std::string& label, const std::string& name, float width)
{
	BeginColumnLabel(label, width);
	ImGui::Text(name.c_str());
	EndColumnLabel();
}

bool gearbox::componentui::DrawInputText(const std::string& label, std::string& name, float width)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::InputText("##label", &name);
	EndColumnLabel();
	return ret;
}

void gearbox::componentui::DrawCheckbox(const std::string& label, bool& value, float width)
{
	BeginColumnLabel(label, width);
	ImGui::Checkbox("##label", &value);
	EndColumnLabel();
}

void gearbox::componentui::DrawUint32(const std::string& label, uint32_t& value, uint32_t minValue, uint32_t maxValue, bool powerOf2, float width, float speed)
{
	BeginColumnLabel(label, width);
	uint32_t _value = powerOf2 ? mars::Utility::NextPowerOf2(value) : value;
	float _speed = powerOf2 ? float(_value / 2) : speed;
	ImGui::DragScalar("##label", ImGuiDataType_U32, (void*)&_value, _speed, (const void*)&minValue, (const void*)&maxValue, "%u");
	value = _value;
	EndColumnLabel();
}

void gearbox::componentui::DrawFloat(const std::string& label, float& value, float minValue, float maxValue, float width, float speed)
{
	BeginColumnLabel(label, width);
	ImGui::DragFloat("##label", &value, speed, minValue, maxValue);
	EndColumnLabel();
}

void gearbox::componentui::DrawDouble(const std::string& label, double& value, double minValue, double maxValue, float width, float speed)
{
	BeginColumnLabel(label, width);
	ImGui::DragScalar("##label", ImGuiDataType_Double, (void*)&value, speed, (const void*)&minValue, (const void*)&maxValue, "%.3f");
	EndColumnLabel();
}

void gearbox::componentui::DrawVec3(const std::string& label, Vec3& value, float resetValue, float width, float speed)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize))
	{
		value.x = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##X", &value.x, speed);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
	{
		value.y = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &value.y, speed);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
	{
		value.z = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Z", &value.z, speed);
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
}

void gearbox::componentui::DrawQuat(const std::string& label, Quat& value, float width, float speed)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	double min = -1.0;
	double max = +1.0;

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() - 20.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.4f, 0.4f, 0.45f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.5f, 0.5f, 0.55f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.4f, 0.4f, 0.45f, 1.0f });
	if (ImGui::Button("S", buttonSize))
	{
		value.s = 1.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##S", ImGuiDataType_Double, (void*)&value.s, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("I", buttonSize))
	{
		value.i = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##I", ImGuiDataType_Double, (void*)&value.i, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("J", buttonSize))
	{
		value.j = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##J", ImGuiDataType_Double, (void*)&value.j, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("K", buttonSize))
	{
		value.k = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##K", ImGuiDataType_Double, (void*)&value.k, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
}

ComponentSettingsBit gearbox::componentui::DrawComponentSetting()
{
	ComponentSettingsBit result = ComponentSettingsBit::NONE_BIT;

	ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
	if (ImGui::Button(". . ."))
		ImGui::OpenPopup("ComponentSettings");

	bool deleteComponent = false;
	if (ImGui::BeginPopup("ComponentSettings"))
	{
		if (ImGui::MenuItem("Remove"))
			result |= ComponentSettingsBit::REMOVE_BIT;

		ImGui::EndPopup();
	}

	return result;
}

ImTextureID gearbox::componentui::GetTextureID(const Ref<miru::crossplatform::ImageView>& imageView, Ref<imgui::UIContext>& uiContext, bool resized, ImTextureID previousTextureID)
{
	ImTextureID ImageID = 0;

	if (GraphicsAPI::IsD3D12())
	{
		if (!previousTextureID || resized)
		{
			Ref<miru::d3d12::Image> d3d12ColourImage = ref_cast<miru::d3d12::Image>(imageView->GetCreateInfo().pImage);

			ID3D12Device* device = (ID3D12Device*)uiContext->GetCreateInfo().window->GetDevice();

			UINT handleIncrement = 0;
			if (previousTextureID && resized)
			{
				if (uiContext->m_D3D12GPUHandleHeapOffsets.find(previousTextureID) != uiContext->m_D3D12GPUHandleHeapOffsets.end())
				{
					handleIncrement = uiContext->m_D3D12GPUHandleHeapOffsets[previousTextureID];
				}
			}
			else
			{
				handleIncrement = (device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * uiContext->m_GPUHandleHeapIndex;
				uiContext->m_GPUHandleHeapIndex++;
			}
			
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
			cpuHandle.ptr = uiContext->m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart().ptr + handleIncrement;
			device->CreateShaderResourceView(d3d12ColourImage->m_Image, nullptr, cpuHandle);
			
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
			gpuHandle.ptr = uiContext->m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart().ptr + handleIncrement;

			ImageID = (ImTextureID)(gpuHandle.ptr);

			uiContext->m_D3D12GPUHandleHeapOffsets[ImageID] = handleIncrement;
		}
	}
	else
	{
		if (!previousTextureID || resized)
		{
			Ref<miru::vulkan::ImageView> vkColourImageView = ref_cast<miru::vulkan::ImageView>(imageView);

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
			VkDevice device = *(VkDevice*)uiContext->GetCreateInfo().window->GetDevice();
			if (!uiContext->m_VKSampler)
				vkCreateSampler(device, &sCI, nullptr, &(uiContext->m_VKSampler));

			if (previousTextureID && resized)
			{
				vkFreeDescriptorSets(device, uiContext->m_VulkanDescriptorPool, 1, (VkDescriptorSet*)&previousTextureID);
			}

			ImageID = ImGui_ImplVulkan_AddTexture(uiContext->m_VKSampler, vkColourImageView->m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	if (ImageID == 0)
		ImageID = previousTextureID;

	return ImageID;
}
