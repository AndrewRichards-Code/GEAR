#include "TextureSerialiser.h"
#include "Asset/EditorAssetManager.h"

#include "Graphics/Texture.h"

using namespace gear;
using namespace asset;
using namespace serialiser;
using namespace graphics;

using namespace miru::base;

using namespace YAML;

Ref<Asset> TextureSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	void* device = AssetSerialiser::GetDevice();

	Texture::CreateInfo textureCI;
	textureCI.device = device;

	std::vector<Asset::Handle> externalAssets;

	Sampler::CreateInfo samplerCI;
	samplerCI.device = device;

	AssetFile assetFile(metadata);
	Node data;
	assetFile.Load(data);

	Node textureNode = data["Texture"];
	ToType(textureCI.debugName, textureNode["debugName"]);
	ToType(textureCI.width, textureNode["width"]);
	ToType(textureCI.height, textureNode["height"]);
	ToType(textureCI.depth, textureNode["depth"]);
	ToType(textureCI.mipLevels, textureNode["mipLevels"]);
	ToType(textureCI.arrayLayers, textureNode["arrayLayers"]);
	ToTypeEnum(textureCI.type, textureNode["type"]);
	ToTypeEnum(textureCI.format, textureNode["format"]);
	ToTypeEnum(textureCI.samples, textureNode["samples"]);
	ToTypeEnum(textureCI.usage, textureNode["usage"]);
	ToType(textureCI.generateMipMaps, textureNode["generateMipMaps"]);
	ToTypeEnum(textureCI.gammaSpace, textureNode["gammaSpace"]);

	Node externalAssetsNode = data["ExternalAssets"];
	size_t i = 0;
	for (const Node& externalAssetNode : externalAssetsNode)
	{
		Ref<ImageAssetDataBuffer> externalAsset = ToAsset<ImageAssetDataBuffer>(externalAssetsNode);

		if (textureCI.width != externalAsset->width
			|| textureCI.height != externalAsset->height
			|| textureCI.depth != externalAsset->depth)
		{
			GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "External Asset's (%ui) dimension {%ui, %ui, %ui} doesn't match the texture asset's dimension {%ui, %ui, %ui}.",
				handle.AsUint64_t(),
				externalAsset->width,
				externalAsset->height,
				externalAsset->depth,
				textureCI.width,
				textureCI.height,
				textureCI.depth);
			continue;
		}

		uint32_t formatSize = Image::GetFormatSize(textureCI.format);
		uint32_t components = Image::GetFormatComponents(textureCI.format);

		size_t pixelCount = textureCI.width * textureCI.height;
		size_t subpixelCount = pixelCount * components;
		size_t layerSize = pixelCount * formatSize;
		bool hdr = (externalAsset->format == Image::Format::R32G32B32A32_SFLOAT);

		std::vector<uint8_t>& imageData = textureCI.imageData;
		if (i == 0)
			imageData.resize(textureCI.arrayLayers * layerSize);

		if (hdr)
		{
			memcpy(imageData.data() + (i * layerSize), externalAsset->Data.data(), layerSize);
		}
		else
		{
			if (textureCI.gammaSpace == GammaSpace::LINEAR)
			{
				//Copy and convert data to floats in linear space
				std::vector<uint8_t> linearImageData(subpixelCount);
				for (size_t i = 0; i < linearImageData.size(); i += 4)
				{
					const std::vector<uint8_t>& buffer = externalAsset->Data;

					Colour_sRGB sRGBColour(buffer[i + 0], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
					Colour_sRGB::Colour_Linear_sRGB lsRGBColour = sRGBColour.Linearise_LUT();

					linearImageData[i + 0] = static_cast<uint8_t>(lsRGBColour.r * static_cast<float>(std::numeric_limits<uint8_t>::max()));
					linearImageData[i + 1] = static_cast<uint8_t>(lsRGBColour.g * static_cast<float>(std::numeric_limits<uint8_t>::max()));
					linearImageData[i + 2] = static_cast<uint8_t>(lsRGBColour.b * static_cast<float>(std::numeric_limits<uint8_t>::max()));
					linearImageData[i + 3] = static_cast<uint8_t>(lsRGBColour.a * static_cast<float>(std::numeric_limits<uint8_t>::max()));
				}

				memcpy(imageData.data() + (i * (imageData.size() / textureCI.arrayLayers)), linearImageData.data(), (imageData.size() / textureCI.arrayLayers));
			}
			else if (textureCI.gammaSpace == GammaSpace::SRGB)
			{
				memcpy(imageData.data() + (i * layerSize), externalAsset->Data.data(), layerSize);
			}
			else
			{
				GEAR_FATAL(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "Unknown ColourSpace.");
			}
		}
		i++;
	}

	Ref<Texture> texture = CreateRef<Texture>(&textureCI);
	texture->handle = handle;

	Node samplerNode = data["Sampler"];
	ToType(samplerCI.debugName, samplerNode["debugName"]);
	ToTypeEnum(samplerCI.magFilter, samplerNode["magFilter"]);
	ToTypeEnum(samplerCI.minFilter, samplerNode["minFilter"]);
	ToTypeEnum(samplerCI.mipmapMode, samplerNode["mipmapMode"]);
	ToTypeEnum(samplerCI.addressModeU, samplerNode["addressModeU"]);
	ToTypeEnum(samplerCI.addressModeV, samplerNode["addressModeV"]);
	ToTypeEnum(samplerCI.addressModeW, samplerNode["addressModeW"]);
	ToType(samplerCI.mipLodBias, samplerNode["mipLodBias"]);
	ToType(samplerCI.anisotropyEnable, samplerNode["anisotropyEnable"]);
	ToType(samplerCI.maxAnisotropy, samplerNode["maxAnisotropy"]);
	ToType(samplerCI.compareEnable, samplerNode["compareEnable"]);
	ToTypeEnum(samplerCI.compareOp, samplerNode["compareOp"]);
	ToType(samplerCI.minLod, samplerNode["minLod"]);
	ToType(samplerCI.maxLod, samplerNode["maxLod"]);
	ToTypeEnum(samplerCI.borderColour, samplerNode["borderColour"]);
	ToType(samplerCI.unnormalisedCoordinates, samplerNode["unnormalisedCoordinates"]);
	texture->SetSampler(&samplerCI);

	texture->externalAssets = externalAssets;

	return ref_cast<Asset>(texture);
}

void TextureSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	AssetFile assetFile(metadata);

	const Ref<Texture>& texture = ref_cast<Texture>(asset);
	const Texture::CreateInfo& textureCI = texture->GetCreateInfo();
	const Sampler::CreateInfo samplerCI = texture->GetSampler()->GetCreateInfo();

	Emitter data;
	data << BeginMap;
	{
		data << Key << "Texture" << Value << BeginSeq;
		{
			data << BeginMap;
			{
				data << Key << "debugName" << Value << textureCI.debugName;
				data << Key << "width" << Value << textureCI.width;
				data << Key << "height" << Value << textureCI.height;
				data << Key << "depth" << Value << textureCI.depth;
				data << Key << "mipLevels" << Value << textureCI.mipLevels;
				data << Key << "arrayLayers" << Value << textureCI.arrayLayers;
				data << Key << "type" << Value << ToString(textureCI.type);
				data << Key << "format" << Value << ToString(textureCI.format);
				data << Key << "samples" << Value << ToString(textureCI.samples);
				data << Key << "usage" << Value << ToString(textureCI.usage);
				data << Key << "generateMipMaps" << Value << textureCI.generateMipMaps;
				data << Key << "gammaSpace" << Value << ToString(textureCI.gammaSpace);
			}
			data << EndMap;
		}
		data << EndSeq;
	}
	data << EndMap;

	data << BeginMap;
	{
		data << Key << "ExternalAssets" << Value << BeginSeq;
		{
			for (size_t i = 0; i < texture->externalAssets.size(); i++)
			{
				data << BeginMap;
				data << Key << i << Value << texture->externalAssets[i];
				data << EndMap;
			}
		}
		data << EndSeq;
	}
	data << EndMap;

	data << BeginMap;
	{
		data << Key << "Sampler" << Value << BeginSeq;
		{
			data << BeginMap;
			{
				data << Key << "debugName" << Value << samplerCI.debugName;
				data << Key << "magFilter" << Value << ToString(samplerCI.magFilter);
				data << Key << "minFilter" << Value << ToString(samplerCI.minFilter);
				data << Key << "mipmapMode" << Value << ToString(samplerCI.mipmapMode);
				data << Key << "addressModeU" << Value << ToString(samplerCI.addressModeU);
				data << Key << "addressModeV" << Value << ToString(samplerCI.addressModeV);
				data << Key << "addressModeW" << Value << ToString(samplerCI.addressModeW);
				data << Key << "mipLodBias" << Value << samplerCI.mipLodBias;
				data << Key << "anisotropyEnable" << Value << samplerCI.anisotropyEnable;
				data << Key << "maxAnisotropy" << Value << samplerCI.maxAnisotropy;
				data << Key << "compareEnable" << Value << samplerCI.compareEnable;
				data << Key << "compareOp" << Value << ToString(samplerCI.compareOp);
				data << Key << "minLod" << Value << samplerCI.minLod;
				data << Key << "maxLod" << Value << samplerCI.maxLod;
				data << Key << "borderColour" << Value << ToString(samplerCI.borderColour);
				data << Key << "unnormalisedCoordinates" << Value << samplerCI.unnormalisedCoordinates;
			}
			data << EndMap;
		}
		data << EndSeq;
	}
	data << EndMap;

	assetFile.Save(data);
}