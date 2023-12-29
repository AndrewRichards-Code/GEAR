#include "gear_core_common.h"
#include "Graphics/Texture.h"
#include "Graphics/AllocatorManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace base;

Texture::Texture(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	//Check cubemap and depth.
	m_Cubemap = m_CI.type == Image::Type::TYPE_CUBE || m_CI.type == Image::Type::TYPE_CUBE_ARRAY;
	m_DepthTexture = m_CI.format >= Image::Format::D16_UNORM;
	if (m_Cubemap && m_CI.arrayLayers % 6 != 0)
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "For TYPE_CUBE or TYPE_CUBE_ARRAY, arrayLayers must be a multiple of 6");
	}

	//Load data
	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	//Calculate Mipmap details
	if (!mars::Utility::IsPowerOf2(m_Width) && !mars::Utility::IsPowerOf2(m_Height))
	{
		m_CI.mipLevels = 1;
		m_CI.generateMipMaps = false;
	}
	else
	{
		uint32_t maxLevels = static_cast<uint32_t>(log2(static_cast<double>(std::min(m_Width, m_Height)))) + 1;
		m_CI.mipLevels = std::min(maxLevels, m_CI.mipLevels);
	}
	m_GenerateMipMaps = m_CI.mipLevels > 1 && m_CI.generateMipMaps;

	//Upload buffer
	m_ImageUploadBufferCI.debugName = "GEAR_CORE_TextureUploadBuffer: " + m_CI.debugName;
	m_ImageUploadBufferCI.device = m_CI.device;
	m_ImageUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::TRANSFER_DST_BIT;
	m_ImageUploadBufferCI.imageDimension = { m_Width, m_Height, 4 };
	m_ImageUploadBufferCI.size = imageData.size();
	m_ImageUploadBufferCI.data = imageData.data();
	m_ImageUploadBufferCI.allocator = AllocatorManager::GetCPUAllocator();
	m_ImageUploadBuffer = Buffer::Create(&m_ImageUploadBufferCI);

	m_ImageUploadBufferViewCI.debugName = "GEAR_CORE_TextureUploadBufferView: " + m_CI.debugName;
	m_ImageUploadBufferViewCI.device = m_CI.device;
	m_ImageUploadBufferViewCI.type = BufferView::Type::UNIFORM_TEXEL;
	m_ImageUploadBufferViewCI.buffer = m_ImageUploadBuffer;
	m_ImageUploadBufferViewCI.offset = 0;
	m_ImageUploadBufferViewCI.size = imageData.size();
	m_ImageUploadBufferViewCI.stride = 0;
	m_ImageUploadBufferView = BufferView::Create(&m_ImageUploadBufferViewCI);

	//Image
	m_TextureCI.debugName = "GEAR_CORE_Texture: " + m_CI.debugName;
	m_TextureCI.device = m_CI.device;
	m_TextureCI.type = m_CI.type;
	m_TextureCI.format = m_CI.format;
	m_TextureCI.width = m_Width;
	m_TextureCI.height = m_Height;
	m_TextureCI.depth = m_Depth;
	m_TextureCI.mipLevels = m_CI.mipLevels;
	m_TextureCI.arrayLayers = m_CI.arrayLayers;
	m_TextureCI.sampleCount = m_CI.samples;
	m_TextureCI.usage = m_CI.usage | Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::TRANSFER_SRC_BIT | (m_GenerateMipMaps ? Image::UsageBit::STORAGE_BIT : Image::UsageBit(0));
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = 0;
	m_TextureCI.data = nullptr;
	m_TextureCI.allocator = AllocatorManager::GetGPUAllocator();
	m_TextureCI.externalImage = nullptr;
	m_Image = Image::Create(&m_TextureCI);

	//ImageView
	m_TextureImageViewCI.debugName = "GEAR_CORE_TextureImageView: " + m_CI.debugName;
	m_TextureImageViewCI.device = m_CI.device;
	m_TextureImageViewCI.image = m_Image;
	m_TextureImageViewCI.viewType = m_CI.type;
	m_TextureImageViewCI.subresourceRange.aspect = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
	m_TextureImageViewCI.subresourceRange.baseMipLevel = 0;
	m_TextureImageViewCI.subresourceRange.mipLevelCount = m_CI.mipLevels;
	m_TextureImageViewCI.subresourceRange.baseArrayLayer = 0;
	m_TextureImageViewCI.subresourceRange.arrayLayerCount = m_CI.arrayLayers;
	m_ImageView = ImageView::Create(&m_TextureImageViewCI);

	//Default sampler
	CreateSampler();
}

Texture::~Texture()
{

}

void Texture::Reload()
{
	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	m_ImageUploadBufferCI.allocator->SubmitData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());

	m_GeneratedMipMaps = false;
}

void Texture::SubmitImageData(std::vector<uint8_t>& imageData)
{
	m_ImageUploadBufferCI.allocator->SubmitData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());
}

void Texture::AccessImageData(std::vector<uint8_t>& imageData)
{
	imageData.clear();
	imageData.resize(m_ImageUploadBuffer->GetAllocation().width);

	m_ImageUploadBufferCI.allocator->AccessData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());
}

void Texture::CreateSampler()
{
	m_SamplerCI.debugName = "GEAR_CORE_Sampler: " + m_CI.debugName;
	m_SamplerCI.device = m_CI.device;
	m_SamplerCI.magFilter = Sampler::Filter::LINEAR;
	m_SamplerCI.minFilter = Sampler::Filter::LINEAR;
	m_SamplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	m_SamplerCI.addressModeU = Sampler::AddressMode::REPEAT;
	m_SamplerCI.addressModeV = Sampler::AddressMode::REPEAT;
	m_SamplerCI.addressModeW = Sampler::AddressMode::REPEAT;
	m_SamplerCI.mipLodBias = 0.0f;
	m_SamplerCI.anisotropyEnable = m_AnisotrophicValue > 1.0f;
	m_SamplerCI.maxAnisotropy = m_AnisotrophicValue;
	m_SamplerCI.compareEnable = false;
	m_SamplerCI.compareOp = CompareOp::NEVER;
	m_SamplerCI.minLod = 0.0f;
	m_SamplerCI.maxLod = static_cast<float>(m_CI.mipLevels);
	m_SamplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	m_SamplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&m_SamplerCI);
}

void Texture::LoadImageData(std::vector<uint8_t>& imageData)
{
	if (m_CI.dataType == DataType::FILE && m_CI.file.filepaths.size())
	{
		m_HDR = stbi_is_hdr(m_CI.file.filepaths[0].c_str());
		uint8_t* stbiBuffer = nullptr;
		float* stbiBufferf = nullptr;

		//Loading an HDR file: set correct parameter
		if (m_HDR && (m_CI.gammaSpace != GammaSpace::LINEAR || m_CI.format != Image::Format::R32G32B32A32_SFLOAT))
		{
			m_CI.gammaSpace = GammaSpace::LINEAR;
			m_CI.format = Image::Format::R32G32B32A32_SFLOAT;
			GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, 
				"Either Texture::CreateInfo::ColourSpace or Texture::CreateInfo::Image::Format was set incorrectly. They are now set to ColourSpace::LINEAR and Image::Format::R32G32B32A32_SFLOAT");
		}
		//Loading a LDR file to linearised: set correct parameter
		if ((!m_HDR && m_CI.gammaSpace == GammaSpace::LINEAR) && m_CI.format != Image::Format::R32G32B32A32_SFLOAT)
		{
			m_CI.format = Image::Format::R32G32B32A32_SFLOAT;
			GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE,
				"Texture::CreateInfo::Image::Format was set incorrectly for ColourSpace::LINEAR. It is now set toImage::Format::R32G32B32A32_SFLOAT");
		}

		for (size_t i = 0; i < m_CI.arrayLayers; i++)
		{
			if (m_HDR)
			{
				stbiBufferf = stbi_loadf(m_CI.file.filepaths[i].c_str(), (int*)&m_Width, (int*)&m_Height, (int*)&m_BPP, 4);
				if (i == 0)
					imageData.resize(m_CI.arrayLayers * m_Width * m_Height * 4 * sizeof(float));

				memcpy(imageData.data() + (i * (imageData.size() / m_CI.arrayLayers)), stbiBufferf, (imageData.size() / m_CI.arrayLayers));
			}
			else
			{
				stbiBuffer = stbi_load(m_CI.file.filepaths[i].c_str(), (int*)&m_Width, (int*)&m_Height, (int*)&m_BPP, 4);
				if (m_CI.gammaSpace == GammaSpace::LINEAR)
				{
					//Copy and convert data to floats in linear space
					std::vector<float> linearImageData(m_Width * m_Height * 4);
					for (size_t i = 0; i < linearImageData.size(); i += 4)
					{
						Colour_sRGB::Colour_Linear_sRGB linearData = Colour_sRGB(stbiBuffer[i + 0], stbiBuffer[i + 1], stbiBuffer[i + 2], stbiBuffer[i + 3]).Linearise_LUT();
						linearImageData[i + 0] = linearData.r;
						linearImageData[i + 1] = linearData.g;
						linearImageData[i + 2] = linearData.b;
						linearImageData[i + 3] = linearData.a;
					}
					if (i == 0)
						imageData.resize(m_CI.arrayLayers * m_Width * m_Height * 4 * sizeof(float));

					memcpy(imageData.data() + (i * (imageData.size() / m_CI.arrayLayers)), linearImageData.data(), (imageData.size() / m_CI.arrayLayers));
				}
				else if (m_CI.gammaSpace == GammaSpace::SRGB)
				{
					
					if (i == 0)
						imageData.resize(m_CI.arrayLayers * m_Width * m_Height * 4 * sizeof(uint8_t));

					memcpy(imageData.data() + (i * (imageData.size() / m_CI.arrayLayers)), stbiBuffer, (imageData.size() / m_CI.arrayLayers));
				}
				else
				{
					GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Unknown ColourSpace.");
				}
			}
		}

		if (stbiBufferf)
			stbi_image_free(stbiBufferf);
		if (stbiBuffer)
			stbi_image_free(stbiBuffer);

		m_Depth = 1;
	}
	else if (m_CI.dataType == Texture::DataType::DATA)
	{
		m_Width = m_CI.data.width;
		m_Height = m_CI.data.height;
		m_Depth = m_CI.data.depth;
		
		if (m_CI.data.data && m_CI.data.size)
		{
			imageData.resize(m_CI.data.size);
			memcpy(imageData.data(), m_CI.data.data, m_CI.data.size);
			m_CI.data.data = nullptr;
		}
		else
		{
			imageData.resize(m_CI.arrayLayers * m_Width * m_Height * 4 * sizeof(uint8_t)); //TODO: Channel and data type should based on the format.
		}
	}
	else
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Unknown Texture::CreateInfo::DataType and/or invalid parameters.");
	}

	if (m_Width == 0 && m_Height == 0 && m_Depth == 0)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Invalid extent for Texture creation.");
	}
}
