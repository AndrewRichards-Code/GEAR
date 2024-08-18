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

	ImTextureID textureID = UIContext::GetUIContext()->AddTextureID(imageView);
	ImGui::Image(textureID, ImVec2(CI.width, CI.height));
}