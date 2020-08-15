#include "gear_core_common.h"
#include "stb_image.h"
#include "Texture.h"
#include "Graphics/MemoryBlockManager.h"

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
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = m_Cubemap ? 6 : 1;
	m_TextureCI.sampleCount = m_CI.samples;
	m_TextureCI.usage = Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT;
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = 0;
	m_TextureCI.data = nullptr;
	m_TextureCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);// , MemoryBlock::BlockSize::BLOCK_SIZE_128MB);
	m_Texture = Image::Create(&m_TextureCI);

	m_InitialBarrierCI.type = Barrier::Type::IMAGE;
	m_InitialBarrierCI.srcAccess = Barrier::AccessBit::NONE;
	m_InitialBarrierCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_InitialBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_InitialBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_InitialBarrierCI.pImage = m_Texture;
	m_InitialBarrierCI.oldLayout = Image::Layout::UNKNOWN;
	m_InitialBarrierCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_InitialBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_InitialBarrier = Barrier::Create(&m_InitialBarrierCI);

	m_ToShaderReadOnlyBarrierCI.type = Barrier::Type::IMAGE;
	m_ToShaderReadOnlyBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_ToShaderReadOnlyBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_ToShaderReadOnlyBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_ToShaderReadOnlyBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_ToShaderReadOnlyBarrierCI.pImage = m_Texture;
	m_ToShaderReadOnlyBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_ToShaderReadOnlyBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_ToShaderReadOnlyBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_ToShaderReadOnlyBarrier = Barrier::Create(&m_ToShaderReadOnlyBarrierCI);

	m_ToTransferDstBarrierCI.type = Barrier::Type::IMAGE;
	m_ToTransferDstBarrierCI.srcAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_ToTransferDstBarrierCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_ToTransferDstBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_ToTransferDstBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_ToTransferDstBarrierCI.pImage = m_Texture;
	m_ToTransferDstBarrierCI.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_ToTransferDstBarrierCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_ToTransferDstBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_ToTransferDstBarrier = Barrier::Create(&m_ToTransferDstBarrierCI);

	m_TextureImageViewCI.debugName = "GEAR_CORE_TextureImageView: " + m_CI.debugName;
	m_TextureImageViewCI.device = m_CI.device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
}

Texture::~Texture()
{
}

void Texture::GetInitialTransition(std::vector<Ref<Barrier>>& barriers, bool force)
{
	if (!m_InitialTransition|| force)
	{
		barriers.push_back(m_InitialBarrier);
		m_InitialTransition = true;
	}
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

void Texture::GetTransition_ToShaderReadOnly(std::vector<Ref<Barrier>>& barriers, bool force)
{
	if (!m_Transition_ToShaderReadOnly || force)
	{
		barriers.push_back(m_ToShaderReadOnlyBarrier);
		m_Transition_ToShaderReadOnly = true;
	}
}

void Texture::GetTransition_ToTransferDst(std::vector<Ref<Barrier>>& barriers, bool force)
{
	if (!m_Transition_ToTransferDst || force)
	{
		barriers.push_back(m_ToTransferDstBarrier);
		m_Transition_ToTransferDst = true;
	}
}

void Texture::Reload()
{
	std::vector<uint8_t> imageData;
	LoadImageData(imageData);

	m_TextureUploadBufferCI.pMemoryBlock->SubmitData(m_TextureUploadBuffer->GetResource(), imageData.size(), imageData.data());

	m_Upload = false;
	m_Transition_ToShaderReadOnly = false;
	m_Transition_ToTransferDst = false;
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
		uint8_t* stbiBuffer = nullptr;
		if (!m_Cubemap)
		{
			stbiBuffer = stbi_load(m_CI.filepaths[0].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);

			imageData.resize(m_CI.width * m_CI.height * 4);
			memcpy(imageData.data(), stbiBuffer, m_CI.width * m_CI.height * 4);

			if (stbiBuffer)
				stbi_image_free(stbiBuffer);

		}
		else
		{
			for (size_t i = 0; i < 6; i++)
			{
				stbiBuffer = stbi_load(m_CI.filepaths[i].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
				if (i == 0)
					imageData.resize(6 * m_CI.width * m_CI.height * 4);

				memcpy(imageData.data() + (i * m_CI.width * m_CI.height * 4), stbiBuffer, (m_CI.width * m_CI.height * 4));

				if (stbiBuffer)
					stbi_image_free(stbiBuffer);
			}

		}
		m_CI.depth = 1;
	}
	else
	{
		imageData.resize(m_CI.size);
		memcpy(imageData.data(), m_CI.data, m_CI.size);
	}
}
