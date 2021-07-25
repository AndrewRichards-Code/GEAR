#include "gear_core_common.h"
#include "stb_image.h"
#include "Texture.h"
#include "Graphics/AllocatorManager.h"
#include "ImageProcessing.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Texture::Texture(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	//Check cubemap and depth.
	m_Cubemap = m_CI.type == Image::Type::TYPE_CUBE || m_CI.type == Image::Type::TYPE_CUBE_ARRAY;
	m_DepthTexture = m_CI.format >= Image::Format::D16_UNORM;
	if (m_Cubemap && m_CI.arrayLayers % 6 != 0)
	{
		GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "For TYPE_CUBE or TYPE_CUBE_ARRAY, arrayLayers must be a multiple of 6");
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
	m_TextureUploadBufferCI.debugName = "GEAR_CORE_TextureUploadBuffer: " + m_CI.debugName;
	m_TextureUploadBufferCI.device = m_CI.device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::TRANSFER_DST_BIT;
	m_TextureUploadBufferCI.imageDimension = { m_Width, m_Height, 4 };
	m_TextureUploadBufferCI.size = imageData.size();
	m_TextureUploadBufferCI.data = imageData.data();
	m_TextureUploadBufferCI.pAllocator = AllocatorManager::GetAllocator(AllocatorManager::AllocatorType::CPU);
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

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
	m_TextureCI.pAllocator = AllocatorManager::GetAllocator(AllocatorManager::AllocatorType::GPU);
	m_Texture = Image::Create(&m_TextureCI);

	//ImageView
	m_TextureImageViewCI.debugName = "GEAR_CORE_TextureImageView: " + m_CI.debugName;
	m_TextureImageViewCI.device = m_CI.device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.viewType = m_CI.type;
	m_TextureImageViewCI.subresourceRange.aspect = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
	m_TextureImageViewCI.subresourceRange.baseMipLevel = 0;
	m_TextureImageViewCI.subresourceRange.mipLevelCount = m_CI.mipLevels;
	m_TextureImageViewCI.subresourceRange.baseArrayLayer = 0;
	m_TextureImageViewCI.subresourceRange.arrayLayerCount = m_CI.arrayLayers;
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	//Default sampler
	CreateSampler();

	//Layouts
	for (uint32_t mipLevel = 0; mipLevel < m_TextureCI.mipLevels; mipLevel++)
	{
		for (uint32_t arrayLayers = 0; arrayLayers < m_TextureCI.arrayLayers; arrayLayers++)
		{
			m_SubresourceMap[mipLevel][arrayLayers] = Image::Layout::UNKNOWN;
		}
	}
}

Texture::~Texture()
{
}

void Texture::Upload(const Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		std::vector<Image::BufferImageCopy> bics;
		for (size_t level = 0; level < 1/*m_CI.mipLevels*/; level++)
		{
			Image::BufferImageCopy bic;
			bic.bufferOffset = level == 0 ? 0 : (m_CI.arrayLayers * (m_Width >> (level - 1)) * (m_Height >> (level - 1)) * 4) + bics.back().bufferOffset;
			bic.bufferRowLength = 0;
			bic.bufferImageHeight = 0;
			bic.imageSubresource.aspectMask = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
			bic.imageSubresource.mipLevel = (uint32_t)level;
			bic.imageSubresource.baseArrayLayer = 0;
			bic.imageSubresource.arrayLayerCount = (uint32_t)m_CI.arrayLayers;
			bic.imageOffset = { 0, 0, 0 };
			bic.imageExtent = { m_Width >> level, m_Height >> level, m_Depth };
			bics.push_back(bic);
		}

		cmdBuffer->CopyBufferToImage(cmdBufferIndex, m_TextureUploadBuffer, m_Texture, Image::Layout::TRANSFER_DST_OPTIMAL, bics);
		m_Upload = true;
	}
}

void Texture::Download(const Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (m_Upload || force)
	{
		std::vector<Image::BufferImageCopy> bics;
		for (size_t level = 0; level < m_CI.mipLevels; level++)
		{
			Image::BufferImageCopy bic;
			bic.bufferOffset = level == 0 ? 0 : (m_CI.arrayLayers * (m_Width >> (level - 1)) * (m_Height >> (level - 1)) * 4) + bics.back().bufferOffset;
			bic.bufferRowLength = 0;
			bic.bufferImageHeight = 0;
			bic.imageSubresource.aspectMask = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
			bic.imageSubresource.mipLevel = (uint32_t)level;
			bic.imageSubresource.baseArrayLayer = 0;
			bic.imageSubresource.arrayLayerCount = (uint32_t)m_CI.arrayLayers;
			bic.imageOffset = { 0, 0, 0 };
			bic.imageExtent = { m_Width >> level, m_Height >> level , m_Depth };
			bics.push_back(bic);
		}
		
		cmdBuffer->CopyImageToBuffer(cmdBufferIndex, m_Texture, m_TextureUploadBuffer, Image::Layout::TRANSFER_SRC_OPTIMAL, bics);
		m_Upload = false;
	}
}

void Texture::TransitionSubResources(std::vector<Ref<Barrier>>& barriers, const std::vector<SubresouresTransitionInfo>& transitionInfos)
{
	Barrier::CreateInfo barrierCI;
	for (auto& transitionInfo : transitionInfos)
	{
		barrierCI.type = Barrier::Type::IMAGE;
		barrierCI.srcAccess = transitionInfo.srcAccess;
		barrierCI.dstAccess = transitionInfo.dstAccess;
		barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.pImage = m_Texture;
		barrierCI.oldLayout = transitionInfo.oldLayout;
		barrierCI.newLayout = transitionInfo.newLayout;
		
		if (transitionInfo.allSubresources)
			barrierCI.subresoureRange = m_TextureImageViewCI.subresourceRange;
		else
			barrierCI.subresoureRange = transitionInfo.subresoureRange;
		
		barriers.emplace_back(Barrier::Create(&barrierCI));
		
		const Image::SubresourceRange& range = barrierCI.subresoureRange;
		for (uint32_t mipLevel = range.baseMipLevel; mipLevel < range.baseMipLevel + range.mipLevelCount; mipLevel++)
		{
			for (uint32_t arrayLayers = range.baseArrayLayer; arrayLayers < range.baseArrayLayer + range.arrayLayerCount; arrayLayers++)
			{
				m_SubresourceMap[mipLevel][arrayLayers] = transitionInfo.newLayout;
			}
		}
	}
}

void Texture::Reload()
{
	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	m_TextureUploadBufferCI.pAllocator->SubmitData(m_TextureUploadBuffer->GetAllocation(), imageData.size(), imageData.data());

	m_Upload = false;
	m_Generated = false;
}

void Texture::SubmitImageData(std::vector<uint8_t>& imageData)
{
	m_TextureUploadBufferCI.pAllocator->SubmitData(m_TextureUploadBuffer->GetAllocation(), imageData.size(), imageData.data());
}

#include "directx12/D3D12Buffer.h"
#include "vulkan/VKBuffer.h"

void Texture::AccessImageData(std::vector<uint8_t>& imageData)
{
	imageData.clear();

	if (GraphicsAPI::IsD3D12())
		imageData.resize((size_t)ref_cast<d3d12::Buffer>(m_TextureUploadBuffer)->m_D3D12MAllocation->GetSize());
	else
		imageData.resize((size_t)ref_cast<vulkan::Buffer>(m_TextureUploadBuffer)->m_VmaAI.size);

	m_TextureUploadBufferCI.pAllocator->AccessData(m_TextureUploadBuffer->GetAllocation(), imageData.size(), imageData.data());
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
	if (m_CI.dataType == DataType::FILE && m_CI.file.filepaths && m_CI.file.count)
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
						mars::Vec4 linearData = Colour_sRGB(stbiBuffer[i + 0], stbiBuffer[i + 1], stbiBuffer[i + 2], stbiBuffer[i + 3]).Linearise_LUT();
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
					GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Unknown ColourSpace.");
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
		}
		else
		{
			imageData.resize(m_CI.arrayLayers * m_Width * m_Height * 4 * sizeof(uint8_t)); //TODO: Channel and data type should based on the format.
		}
	}
	else
	{
		GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Unknown Texture::CreateInfo::DataType and/or invalid parameters.");
	}

	if (m_Width == 0 && m_Height == 0 && m_Depth == 0)
	{
		GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Invalid extent for Texture creation.");
	}
}
