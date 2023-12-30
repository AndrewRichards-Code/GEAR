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
	Buffer::CreateInfo imageUploadBufferCI;
	imageUploadBufferCI.debugName = "GEAR_CORE_TextureUploadBuffer: " + m_CI.debugName;
	imageUploadBufferCI.device = m_CI.device;
	imageUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::TRANSFER_DST_BIT;
	imageUploadBufferCI.imageDimension = { m_Width, m_Height, std::max(m_Depth, m_CI.arrayLayers), Image::GetFormatSize(m_CI.format) };
	imageUploadBufferCI.size = imageData.size();
	imageUploadBufferCI.data = imageData.data();
	imageUploadBufferCI.allocator = AllocatorManager::GetCPUAllocator();
	m_ImageUploadBuffer = Buffer::Create(&imageUploadBufferCI);

	BufferView::CreateInfo imageUploadBufferViewCI;
	imageUploadBufferViewCI.debugName = "GEAR_CORE_TextureUploadBufferView: " + m_CI.debugName;
	imageUploadBufferViewCI.device = m_CI.device;
	imageUploadBufferViewCI.type = BufferView::Type::UNIFORM_TEXEL;
	imageUploadBufferViewCI.buffer = m_ImageUploadBuffer;
	imageUploadBufferViewCI.offset = 0;
	imageUploadBufferViewCI.size = m_ImageUploadBuffer->GetCreateInfo().size;
	imageUploadBufferViewCI.stride = 0;
	m_ImageUploadBufferView = BufferView::Create(&imageUploadBufferViewCI);

	//Image
	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_CORE_Texture: " + m_CI.debugName;
	imageCI.device = m_CI.device;
	imageCI.type = m_CI.type;
	imageCI.format = m_CI.format;
	imageCI.width = m_Width;
	imageCI.height = m_Height;
	imageCI.depth = m_Depth;
	imageCI.mipLevels = m_CI.mipLevels;
	imageCI.arrayLayers = m_CI.arrayLayers;
	imageCI.sampleCount = m_CI.samples;
	imageCI.usage = m_CI.usage | Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::TRANSFER_SRC_BIT | (m_GenerateMipMaps ? Image::UsageBit::STORAGE_BIT : Image::UsageBit(0));
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.allocator = AllocatorManager::GetGPUAllocator();
	imageCI.externalImage = nullptr;
	m_Image = Image::Create(&imageCI);

	//ImageView
	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "GEAR_CORE_TextureImageView: " + m_CI.debugName;
	imageViewCI.device = m_CI.device;
	imageViewCI.image = m_Image;
	imageViewCI.viewType = m_CI.type;
	imageViewCI.subresourceRange.aspect = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.mipLevelCount = m_CI.mipLevels;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.arrayLayerCount = m_CI.arrayLayers;
	m_ImageView = ImageView::Create(&imageViewCI);

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

	m_ImageUploadBuffer->GetCreateInfo().allocator->SubmitData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());

	m_GeneratedMipMaps = false;
}

void Texture::SubmitImageData(std::vector<uint8_t>& imageData)
{
	m_ImageUploadBuffer->GetCreateInfo().allocator->SubmitData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());
}

void Texture::AccessImageData(std::vector<uint8_t>& imageData)
{
	imageData.clear();
	imageData.resize(m_ImageUploadBuffer->GetCreateInfo().size);

	m_ImageUploadBuffer->GetCreateInfo().allocator->AccessData(m_ImageUploadBuffer->GetAllocation(), 0, imageData.size(), imageData.data());
}

void Texture::CreateSampler()
{
	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "GEAR_CORE_Sampler: " + m_CI.debugName;
	samplerCI.device = m_CI.device;
	samplerCI.magFilter = Sampler::Filter::LINEAR;
	samplerCI.minFilter = Sampler::Filter::LINEAR;
	samplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	samplerCI.addressModeU = Sampler::AddressMode::REPEAT;
	samplerCI.addressModeV = Sampler::AddressMode::REPEAT;
	samplerCI.addressModeW = Sampler::AddressMode::REPEAT;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.anisotropyEnable = m_AnisotrophicValue > 1.0f;
	samplerCI.maxAnisotropy = m_AnisotrophicValue;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(m_CI.mipLevels);
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&samplerCI);
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
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Invalid extent for Texture creation.");
	}
}
