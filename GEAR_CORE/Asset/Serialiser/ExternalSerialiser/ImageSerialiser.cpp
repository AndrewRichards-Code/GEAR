#include "ImageSerialiser.h"

#include "Asset/AssetDataBuffer.h"

#include "MIRU/MIRU_CORE/src/base/Image.h"
#include "ARC/src/FileLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

using namespace gear;
using namespace asset;
using namespace serialiser;

using namespace miru::base;

Ref<Asset> ImageSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	bool hdr = stbi_is_hdr(metadata.filepath.generic_string().c_str());

	Ref<ImageAssetDataBuffer> asset = CreateRef<ImageAssetDataBuffer>();
	asset->format = hdr ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::R8G8B8A8_UNORM;

	uint32_t channels = 0;
	uint32_t formatSize = Image::GetFormatSize(asset->format);
	uint32_t components = Image::GetFormatComponents(asset->format);

	void* stbiBuffer = nullptr;

	if (hdr)
	{
		stbiBuffer = stbi_loadf(metadata.filepath.generic_string().c_str(), (int*)&asset->width, (int*)&asset->height, (int*)&channels, components);
	}
	else
	{
		stbiBuffer = stbi_load(metadata.filepath.generic_string().c_str(), (int*)&asset->width, (int*)&asset->height, (int*)&channels, components);
	}
	
	if (!stbiBuffer && metadata.filepath.extension() == ".ico")
	{
		LoadICOData(metadata, stbiBuffer, asset->width, asset->height, channels, components);
	}

	asset->depth = 1;
	asset->Data.resize(asset->width * asset->height * formatSize);
	memcpy_s(asset->Data.data(), asset->Data.size(), stbiBuffer, asset->Data.size());

	if (stbiBuffer)
		stbi_image_free(stbiBuffer);

	asset->handle = handle;
	return asset;
}

void ImageSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	if (metadata.CheckFilepathExtension(".png"))
	{
		Ref<ImageAssetDataBuffer> imageDataAsset = ref_cast<ImageAssetDataBuffer>(asset);
		stbi_write_png(metadata.filepath.generic_string().c_str(),
			(int)imageDataAsset->width, (int)imageDataAsset->height, 4,
			imageDataAsset->Data.data(), (int)imageDataAsset->width * 4);
	}
	if (metadata.CheckFilepathExtension(".tga"))
	{
		Ref<ImageAssetDataBuffer> imageDataAsset = ref_cast<ImageAssetDataBuffer>(asset);
		stbi_write_tga(metadata.filepath.generic_string().c_str(),
			(int)imageDataAsset->width, (int)imageDataAsset->height, 4,
			imageDataAsset->Data.data());
	}
	if (metadata.CheckFilepathExtension(".bmp"))
	{
		Ref<ImageAssetDataBuffer> imageDataAsset = ref_cast<ImageAssetDataBuffer>(asset);
		stbi_write_bmp(metadata.filepath.generic_string().c_str(),
			(int)imageDataAsset->width, (int)imageDataAsset->height, 4,
			imageDataAsset->Data.data());
	}
	if (metadata.CheckFilepathExtension(".jpeg") || metadata.CheckFilepathExtension(".jpg"))
	{
		Ref<ImageAssetDataBuffer> imageDataAsset = ref_cast<ImageAssetDataBuffer>(asset);
		stbi_write_jpg(metadata.filepath.generic_string().c_str(),
			(int)imageDataAsset->width, (int)imageDataAsset->height, 4,
			imageDataAsset->Data.data(), 100);
	}
	if (metadata.CheckFilepathExtension(".exr") || metadata.CheckFilepathExtension(".hdr"))
	{
		Ref<ImageAssetDataBuffer> imageDataAsset = ref_cast<ImageAssetDataBuffer>(asset);
		stbi_write_hdr(metadata.filepath.generic_string().c_str(),
			(int)imageDataAsset->width, (int)imageDataAsset->height, 4,
			(float*)imageDataAsset->Data.data());
	}
}

void ImageSerialiser::LoadICOData(const AssetMetadata& metadata, void*& stbiBuffer, uint32_t& width, uint32_t& height, uint32_t& channels, uint32_t components)
{
	//Taken from this: https://github.com/nothings/stb/issues/688
	const std::vector<char>& icoFileData = arc::ReadBinaryFile(metadata.filepath);
	if (!icoFileData.empty())
	{
		// Icon File Header (6 bytes)
		struct IcoHeader
		{
			uint16_t reserved;    // Must always be 0.
			uint16_t imageType;   // Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
			uint16_t imageCount;  // Specifies number of images in the file.
		} icoHeader;

		// Icon Entry info (16 bytes)
		struct IcoDirEntry
		{
			uint8_t width;        // Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
			uint8_t height;       // Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
			uint8_t colpalette;   // Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
			uint8_t reserved;     // Reserved. Should be 0.
			uint16_t planes;      // In ICO format: Specifies color planes. Should be 0 or 1. // In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the left.
			uint16_t bpp;         // In ICO format: Specifies bits per pixel. [Notes 4] // In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top. 
			uint32_t size;        // Specifies the size of the image's data in bytes
			uint32_t offset;      // Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
		} icoDirEntry;

		//We assume there is only one image in the .ico file
		memcpy_s(&icoHeader, sizeof(IcoHeader), icoFileData.data(), sizeof(IcoHeader));
		memcpy_s(&icoDirEntry, sizeof(IcoDirEntry), icoFileData.data() + sizeof(IcoHeader), sizeof(IcoDirEntry));

		stbiBuffer = stbi_load_from_memory((uint8_t*)(icoFileData.data() + icoDirEntry.offset), icoDirEntry.size, (int*)&width, (int*)&height, (int*)&channels, components);
	}
}
