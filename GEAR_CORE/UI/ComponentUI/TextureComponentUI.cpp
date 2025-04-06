#include "gear_core_common.h"
#include "TextureComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

using namespace gear;
using namespace graphics;

using namespace ui;
using namespace componentui;

using namespace miru::base;

void gear::ui::componentui::DrawTextureComponentUI(Ref<Texture> texture)
{
	const Texture::CreateInfo& CI = texture->GetCreateInfo();
	const ImageRef& image = texture->GetImage();
	static ImVec2 regionSize = {};

	//View size
	static float scale = 100.0f;
	if (ImGui::Button("Fit") && regionSize.x > 0.0f && regionSize.y > 0.0f)
	{
		float widthScale = static_cast<float>(CI.width) / regionSize.x;
		float heightScale = static_cast<float>(CI.height) / regionSize.y;
		const bool& tooWide = widthScale > 1.0f;
		const bool& tooHigh = heightScale > 1.0f;

		if (tooWide || tooHigh)
		{
			widthScale = tooWide ? (1.0f / widthScale) : widthScale;
			heightScale = tooHigh ? (1.0f / heightScale) : heightScale;
			
			if (tooWide && tooHigh)
				scale = std::min(widthScale, heightScale) * 100.0f;
			else 
				scale = std::max(widthScale, heightScale) * 100.0f;
		}
		else
		{
			const float& minRegionSize = std::min(regionSize.x, regionSize.y);
			const float& maxImageDimension = std::max(static_cast<float>(CI.width), static_cast<float>(CI.height));
			scale = (minRegionSize / maxImageDimension) * 100.0f;
		}
		
	}
	ImGui::SameLine();
	if (ImGui::Button(std::string("100%").c_str()))
	{
		scale = 100.0f;
	}
	ImGui::SameLine();
	DrawFloat("Scale(%%)", scale, 10.0f, 1000.0f);

	//Colour Mask
	static uint8_t colourMask = 0x0F;
	auto UpdateColourMask = [&](uint8_t x)
		{
			if (arc::BitwiseCheck(colourMask, x))
				colourMask &= ~x;
			else
				colourMask |= x;
		};

	const ImVec4& red = arc::BitwiseCheck(colourMask, uint8_t(1)) ? ImVec4(0.8f, 0.1f, 0.15f, 1.0f) : ImVec4(0.4f, 0.05f, 0.075f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, red);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, red);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, red);
	if (ImGui::Button("R"))
	{
		UpdateColourMask(1);
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();

	const ImVec4& green = arc::BitwiseCheck(colourMask, uint8_t(2)) ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.1f, 0.35f, 0.1f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, green);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, green);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, green);
	if (ImGui::Button("G"))
	{
		UpdateColourMask(2);
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();

	const ImVec4& blue = arc::BitwiseCheck(colourMask, uint8_t(4)) ? ImVec4(0.1f, 0.25f, 0.8f, 1.0f) : ImVec4(0.05f, 0.125f, 0.4f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, blue);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blue);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, blue);
	if (ImGui::Button("B"))
	{
		UpdateColourMask(4);
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();

	const ImVec4& alpha = arc::BitwiseCheck(colourMask, uint8_t(8)) ? ImVec4(0.2f, 0.205f, 0.21f, 1.0f) : ImVec4(0.1f, 0.1025f, 0.105f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, alpha);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, alpha);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, alpha);
	if (ImGui::Button("A"))
	{
		UpdateColourMask(8);
	}
	ImGui::PopStyleColor(3);

	//Level and Layer
	bool changed = false;
	static uint32_t mipLevel = 0;
	changed |= DrawUint32("Mip Level", mipLevel, 0, CI.mipLevels);
	mipLevel = std::clamp<uint32_t>(mipLevel, 0, CI.mipLevels - 1);

	static uint32_t arrayLayer = 0;
	changed |= DrawUint32("Array Layer", arrayLayer, 0, CI.arrayLayers);
	arrayLayer = std::clamp<uint32_t>(mipLevel, 0, CI.arrayLayers - 1);

	ImageViewRef imageView = nullptr;
	if (changed || !imageView)
	{
		ImageView::CreateInfo imageViewCI;
		imageViewCI.debugName = image->GetCreateInfo().debugName + "_" + std::to_string(mipLevel) + "_" + std::to_string(arrayLayer);
		imageViewCI.device = image->GetCreateInfo().device;
		imageViewCI.image = image;
		imageViewCI.viewType = texture->GetImageView()->GetCreateInfo().viewType;
		imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, mipLevel, 1, arrayLayer, 1 };
		imageView = ImageView::Create(&imageViewCI);
	}

	const ImVec2& size = ImVec2(static_cast<float>(CI.width) * scale / 100.0f, static_cast<float>(CI.height) * scale / 100.0f);
	const ImVec2& uv0 = ImVec2(0, 0);
	const ImVec2& uv1 = ImVec2(1, 1);
	const ImVec4& tint_col = ImVec4(
		arc::BitwiseCheck(colourMask, uint8_t(1)) ? 1.0f : 0.0f,
		arc::BitwiseCheck(colourMask, uint8_t(2)) ? 1.0f : 0.0f,
		arc::BitwiseCheck(colourMask, uint8_t(4)) ? 1.0f : 0.0f,
		arc::BitwiseCheck(colourMask, uint8_t(8)) ? 1.0f : 0.0f
	);
	ImTextureID textureID = UIContext::GetUIContext()->AddTextureID(imageView);

	regionSize = ImGui::GetContentRegionAvail();
	ImGui::Image(textureID, size, uv0, uv1, tint_col);
}