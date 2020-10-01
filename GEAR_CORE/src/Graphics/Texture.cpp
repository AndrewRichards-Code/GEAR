#include "gear_core_common.h"
#include "stb_image.h"
#include "Texture.h"
#include "MemoryBlockManager.h"
#include "ImageProcessing.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Texture::Texture(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_Cubemap = m_CI.type == Image::Type::TYPE_CUBE;
	m_DepthTexture = m_CI.format >= Image::Format::D16_UNORM;

	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	auto IsPowerOf2 = [](uint32_t x) -> bool { return !(x == 0) && !(x & (x - 1)); };
	if (!(IsPowerOf2(m_CI.width) && IsPowerOf2(m_CI.height)))
	{
		m_CI.mipLevels = 1;
		m_CI.generateMipMaps = false;
	}
	else
	{
		uint32_t maxLevels = static_cast<uint32_t>(log2(static_cast<double>(std::min(m_CI.width, m_CI.height)))) + 1;
		m_CI.mipLevels = std::min(maxLevels, m_CI.mipLevels);
	}
	m_GenerateMipMaps = m_CI.mipLevels > 1 && m_CI.generateMipMaps;

	m_TextureUploadBufferCI.debugName = "GEAR_CORE_TextureUploadBuffer: " + m_CI.debugName;
	m_TextureUploadBufferCI.device = m_CI.device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_TextureUploadBufferCI.imageDimension = { m_CI.width, m_CI.height, 4 };
	m_TextureUploadBufferCI.size = imageData.size();
	m_TextureUploadBufferCI.data = imageData.data();
	m_TextureUploadBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);// , MemoryBlock::BlockSize::BLOCK_SIZE_128MB);
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

	m_TextureCI.debugName = "GEAR_CORE_Texture: " + m_CI.debugName;
	m_TextureCI.device = m_CI.device;
	m_TextureCI.type = m_CI.type;
	m_TextureCI.format = m_CI.format;
	m_TextureCI.width = m_CI.width;
	m_TextureCI.height = m_CI.height;
	m_TextureCI.depth = m_CI.depth;
	m_TextureCI.mipLevels = m_CI.mipLevels;
	m_TextureCI.arrayLayers = m_Cubemap ? 6 : 1;
	m_TextureCI.sampleCount = m_CI.samples;
	m_TextureCI.usage = m_CI.usage | Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT | (m_GenerateMipMaps ? Image::UsageBit::STORAGE_BIT : Image::UsageBit(0));
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = 0;
	m_TextureCI.data = nullptr;
	m_TextureCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);// , MemoryBlock::BlockSize::BLOCK_SIZE_128MB);
	m_Texture = Image::Create(&m_TextureCI);

	for (uint32_t mipLevel = 0; mipLevel < m_TextureCI.mipLevels; mipLevel++)
	{
		for (uint32_t arrayLayers = 0; arrayLayers < m_TextureCI.arrayLayers; arrayLayers++)
		{
			m_SubresourceMap[mipLevel][arrayLayers] = Image::Layout::UNKNOWN;
		}
	}

	m_TextureImageViewCI.debugName = "GEAR_CORE_TextureImageView: " + m_CI.debugName;
	m_TextureImageViewCI.device = m_CI.device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange.aspect = m_DepthTexture ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT;
	m_TextureImageViewCI.subresourceRange.baseMipLevel = 0;
	m_TextureImageViewCI.subresourceRange.mipLevelCount = m_CI.mipLevels;
	m_TextureImageViewCI.subresourceRange.baseArrayLayer = 0;
	m_TextureImageViewCI.subresourceRange.arrayLayerCount = static_cast<uint32_t>(m_Cubemap ? 6 : 1);
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
}

Texture::~Texture()
{
}

void Texture::Upload(const miru::Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		std::vector<Image::BufferImageCopy> bics;
		if (m_Cubemap)
		{
			for (size_t i = 0; i < 6; i++)
			{
				Image::BufferImageCopy bic;
				bic.bufferOffset = (i * m_CI.width * m_CI.height * 4);
				bic.bufferRowLength = 0;
				bic.bufferImageHeight = 0;
				bic.imageSubresource = { Image::AspectBit::COLOUR_BIT, 0, (uint32_t)i, 1 };
				bic.imageOffset = { 0, 0, 0 };
				bic.imageExtent = { m_CI.width, m_CI.height, m_CI.depth };
				bics.push_back(bic);
			}
		}
		else
		{
			Image::BufferImageCopy bic;
			bic.bufferOffset = 0;
			bic.bufferRowLength = 0;
			bic.bufferImageHeight = 0;
			bic.imageSubresource = { Image::AspectBit::COLOUR_BIT, 0, 0, 1 };
			bic.imageOffset = { 0, 0, 0 };
			bic.imageExtent = { m_CI.width, m_CI.height, m_CI.depth };
			bics.push_back(bic);
		}

		cmdBuffer->CopyBufferToImage(cmdBufferIndex, m_TextureUploadBuffer, m_Texture, Image::Layout::TRANSFER_DST_OPTIMAL, bics);
		m_Upload = true;
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

void Texture::GenerateMipMaps()
{
	if (!m_Upload)
	{
		GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_STATE, "Texture data has not been uploaded. Can not generate mipmaps.");
		return;
	}
	if (m_GenerateMipMaps && !m_Generated)
	{
		ImageProcessing::GenerateMipMaps(this);;
		m_Generated = true;
	}
}

void Texture::Reload()
{
	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	m_TextureUploadBufferCI.pMemoryBlock->SubmitData(m_TextureUploadBuffer->GetResource(), imageData.size(), imageData.data());

	m_Upload = false;
	m_Generated = false;
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
	m_SamplerCI.mipLodBias = 1.0f;
	m_SamplerCI.anisotropyEnable = m_AnisotrophicValue > 1.0f;
	m_SamplerCI.maxAnisotropy = m_AnisotrophicValue;
	m_SamplerCI.compareEnable = false;
	m_SamplerCI.compareOp = CompareOp::NEVER;
	m_SamplerCI.minLod = 0.0f;
	m_SamplerCI.maxLod = 1.0f;
	m_SamplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	m_SamplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&m_SamplerCI);
}

void Texture::LoadImageData(std::vector<uint8_t>& imageData)
{
	if (!m_CI.filepaths.empty())
	{
		m_HDR = stbi_is_hdr(m_CI.filepaths[0].c_str());
		uint8_t* stbiBuffer = nullptr;
		float* stbiBufferf = nullptr;
		
		if (!m_Cubemap)
		{
			if (m_HDR)
			{
				stbiBufferf = stbi_loadf(m_CI.filepaths[0].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
				imageData.resize(m_CI.width * m_CI.height * 4 * sizeof(float));
				memcpy(imageData.data(), stbiBufferf, imageData.size());
			}
			else
			{
				stbiBuffer = stbi_load(m_CI.filepaths[0].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
				imageData.resize(m_CI.width * m_CI.height * 4 * sizeof(uint8_t));
				memcpy(imageData.data(), stbiBuffer, imageData.size());
			}
		}
		else
		{
			for (size_t i = 0; i < 6; i++)
			{
				if (m_HDR)
				{
					stbiBufferf = stbi_loadf(m_CI.filepaths[i].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
					if (i == 0) 
						imageData.resize(6 * m_CI.width * m_CI.height * 4 * sizeof(float));

					memcpy(imageData.data() + (i * (imageData.size() / 6)), stbiBufferf, (imageData.size() / 6));
				}
				else
				{
					stbiBuffer = stbi_load(m_CI.filepaths[i].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
					if (i == 0)
						imageData.resize(6 * m_CI.width * m_CI.height * 4 * sizeof(uint8_t));

					memcpy(imageData.data() + (i * (imageData.size() / 6)), stbiBuffer, (imageData.size() / 6));
				}
			}

		}

		if (stbiBufferf)
			stbi_image_free(stbiBufferf);
		if (stbiBuffer)
			stbi_image_free(stbiBuffer);

		m_CI.depth = 1;
	}
	else
	{
		imageData.resize(m_CI.size);
		memcpy(imageData.data(), m_CI.data, m_CI.size);
	}
}
