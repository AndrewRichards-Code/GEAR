#include "ExternalFileSerialiser.h"

#include "Asset/AssetFile.h"
#include "Asset/AssetDataBuffer.h"

#include "Objects/Mesh.h"
#include "Utils/ModelLoader.h"

#include "MIRU/MIRU_CORE/src/base/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

using namespace gear;
using namespace asset;
using namespace serialiser;

using namespace utils;

using namespace miru::base;

Ref<Asset> ExternalFileSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	if (metadata.IsFilepathExtensionImageType())
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
		
		asset->depth = 1;
		asset->Data.resize(asset->width * asset->height * formatSize);
		memcpy(asset->Data.data(), stbiBuffer, asset->Data.size());

		if (stbiBuffer)
			stbi_image_free(stbiBuffer);

		asset->handle = handle;
		return asset;
	}
	else if (metadata.IsFilepathExtensionMeshType())
	{
		ModelLoader::SetDevice(AssetSerialiser::GetDevice());
		Ref<ModelLoader::ModelData> asset = CreateRef<ModelLoader::ModelData>(ModelLoader::LoadModelData(metadata.filepath));
		asset->handle = handle;
		return asset;
	}
	else
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::NOT_SUPPORTED, "Filepath extension: %s is not supported", metadata.filepath.generic_string().c_str());
	}

	return CreateRef<Asset>();
}

void ExternalFileSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	if (metadata.IsFilepathExtensionImageType())
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
}