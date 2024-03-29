#include "gear_core_common.h"
#include "ComponentUI.h"
#include "UI/UIContext.h"

#include "d3d12/D3D12Image.h"
#include "vulkan/VKImage.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui/backends/imgui_impl_vulkan.h"

using namespace gear;
using namespace scene;

using namespace ui;
using namespace componentui;

using namespace miru::base;

using namespace mars;

void gear::ui::componentui::DrawStaticText(const std::string& label, const std::string& name, float width)
{
	BeginColumnLabel(label, width);
	ImGui::Text(name.c_str());
	EndColumnLabel();
}

bool gear::ui::componentui::DrawInputText(const std::string& label, std::string& name, float width)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::InputText("##label", &name);
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawCheckbox(const std::string& label, bool& value, float width)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::Checkbox("##label", &value);
	EndColumnLabel();
	return ret;
}

void gear::ui::componentui::DrawToggleButton(const char* str_id, bool& value)
{
	ImVec4* colours = ImGui::GetStyle().Colors;
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = height * 1.55f;
	float radius = height * 0.50f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked())
		value = !value;
	ImGuiContext& gg = *GImGui;
	float ANIM_SPEED = 0.085f;
	if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(value ? colours[ImGuiCol_ButtonActive] : colours[ImGuiCol_Button]), height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + (value ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, ImGui::GetColorU32(colours[ImGuiCol_Tab]));

	ImGui::SameLine();
	ImGui::Text(str_id);
}


bool gear::ui::componentui::DrawUint32(const std::string& label, uint32_t& value, uint32_t minValue, uint32_t maxValue, bool powerOf2, float width, float speed)
{
	BeginColumnLabel(label, width);
	uint32_t _value = powerOf2 ? Utility::NextPowerOf2(value) : value;
	float _speed = powerOf2 ? float(_value / 2) : speed;
	bool ret = ImGui::DragScalar("##label", ImGuiDataType_U32, (void*)&_value, _speed, (const void*)&minValue, (const void*)&maxValue, "%u");
	value = _value;
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawInt32(const std::string& label, int32_t& value, int32_t minValue, int32_t maxValue, float width, float speed)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::DragScalar("##label", ImGuiDataType_S32, (void*)&value, speed, (const void*)&minValue, (const void*)&maxValue, "%i");
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawFloat(const std::string& label, float& value, float minValue, float maxValue, float width, float speed, const char* format)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::DragScalar("##label", ImGuiDataType_Float, (void*)&value, speed, (const void*)&minValue, (const void*)&maxValue, format ? format : "%.3f");
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawDouble(const std::string& label, double& value, double minValue, double maxValue, float width, float speed, const char* format)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::DragScalar("##label", ImGuiDataType_Double, (void*)&value, speed, (const void*)&minValue, (const void*)&maxValue, format ? format : "%.3f");
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawFloat3(const std::string& label, float3& value, float resetValue, float width, float speed, const char* format)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	bool ret = false;

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize))
	{
		value.x = resetValue;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragFloat("##X", &value.x, speed, 0.0f, 0.0f, format ? format : "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
	{
		value.y = resetValue;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragFloat("##Y", &value.y, speed, 0.0f, 0.0f, format ? format : "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
	{
		value.z = resetValue;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragFloat("##Z", &value.z, speed, 0.0f, 0.0f, format ? format : "%.3f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawQuaternion(const std::string& label, Quaternion& value, float width, float speed, const char* format)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	double min = -1.0;
	double max = +1.0;
	bool ret = false;

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() - 20.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.4f, 0.4f, 0.45f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.5f, 0.5f, 0.55f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.4f, 0.4f, 0.45f, 1.0f });
	if (ImGui::Button("S", buttonSize))
	{
		value.s = 1.0;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragScalar("##S", ImGuiDataType_Double, (void*)&value.s, speed, (const void*)&min, (const void*)&max, format ? format : "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("I", buttonSize))
	{
		value.i = 0.0;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragScalar("##I", ImGuiDataType_Double, (void*)&value.i, speed, (const void*)&min, (const void*)&max, format ? format : "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("J", buttonSize))
	{
		value.j = 0.0;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragScalar("##J", ImGuiDataType_Double, (void*)&value.j, speed, (const void*)&min, (const void*)&max, format ? format : "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("K", buttonSize))
	{
		value.k = 0.0;
		ret |= true;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ret |= ImGui::DragScalar("##K", ImGuiDataType_Double, (void*)&value.k, speed, (const void*)&min, (const void*)&max, format ? format : "%.3f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawColourPicker3(const std::string& label, float3& value, float width)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::ColorPicker3("##ColorPicker3", &value.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB);
	EndColumnLabel();
	return ret;
}

bool gear::ui::componentui::DrawColourPicker4(const std::string& label, float4& value, float width)
{
	BeginColumnLabel(label, width);
	bool ret = ImGui::ColorPicker4("##ColorPicker4", &value.x, ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB);
	EndColumnLabel();
	return ret;
}

void gear::ui::componentui::DrawInputVectorOfString(const std::string& label, std::vector<std::string>& vector, float width)
{
	BeginColumnLabel(label, width);
	for (auto it = vector.begin(); it != vector.end(); it++)
	{
		std::string name = *it;
		ImGui::InputText("##label", &name);
		
		ImGui::SameLine();
		if (ImGui::Button("+"))
		{
			vector.insert(it, "");
			it = vector.begin();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("X"))
		{
			vector.erase(it);
			if (!vector.empty())
				it = vector.begin();
			else
				break;
		}
	}
	EndColumnLabel();
};

ComponentSettingsBit gear::ui::componentui::DrawComponentSetting()
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

ImTextureID gear::ui::componentui::GetTextureID(const miru::base::ImageViewRef& imageView, UIContext* uiContext, bool resized)
{

	bool found = uiContext->m_TextureIDs.find(imageView) != uiContext->m_TextureIDs.end();
	if (found && !resized)
	{
		return uiContext->m_TextureIDs[imageView];
	}
	else
	{
		ImTextureID& ImageID = uiContext->m_TextureIDs[imageView];
		if (GraphicsAPI::IsD3D12())
		{
			const miru::d3d12::ImageRef& d3d12ColourImage = ref_cast<miru::d3d12::Image>(imageView->GetCreateInfo().image);
			const miru::d3d12::ImageViewRef& d3d12ColourImageView = ref_cast<miru::d3d12::ImageView>(imageView);

			ID3D12Device* device = (ID3D12Device*)uiContext->GetDevice();
			UINT handleIncrement = 0;

			//Reuse old heap location
			if (found && resized)
			{
				handleIncrement = uiContext->m_D3D12GPUHandleHeapOffsets[ImageID];
			}
			else
			{
				handleIncrement = (device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * uiContext->m_GPUHandleHeapIndex;
				uiContext->m_GPUHandleHeapIndex++;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
			cpuHandle.ptr = uiContext->m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart().ptr + handleIncrement;
			device->CreateShaderResourceView(d3d12ColourImage->m_Image, &d3d12ColourImageView->m_SRVDesc, cpuHandle);
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
			gpuHandle.ptr = uiContext->m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart().ptr + handleIncrement;

			ImageID = (ImTextureID)(gpuHandle.ptr);
			uiContext->m_D3D12GPUHandleHeapOffsets[ImageID] = handleIncrement;

		}
		else
		{
			VkDevice device = *(VkDevice*)uiContext->GetDevice();
			const miru::vulkan::ImageViewRef& vkColourImageView = ref_cast<miru::vulkan::ImageView>(imageView);

			//Free old descriptor set
			if (found && resized)
			{
				vkFreeDescriptorSets(device, uiContext->m_VulkanDescriptorPool, 1, (VkDescriptorSet*)&ImageID);
			}

			ImageID = (ImTextureID)ImGui_ImplVulkan_AddTexture(uiContext->m_VulkanSampler, vkColourImageView->m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		return ImageID;
	}
}
