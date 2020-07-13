#include "gear_core_common.h"
#include "texture.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> Texture::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> Texture::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> Texture::s_MB_GPU_Usage = nullptr;
uint32_t Texture::s_UID = 0;

Texture::Texture(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseMemory();

	bool loadFromFile = !m_CI.filepaths.empty() && !m_CI.data;
	m_Cubemap = m_CI.type == Image::Type::TYPE_CUBE;
	m_DepthTexture = m_CI.format >= Image::Format::D16_UNORM;
	
	std::vector<uint8_t> imageData;
	
	m_TextureName = "UID: " + std::to_string(s_UID);
	if (loadFromFile)
	{
		m_TextureName = " File: " + m_CI.filepaths[0];
		
		if (!m_Cubemap)
		{
			m_LocalBuffer = stbi_load(m_CI.filepaths[0].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);

			imageData.resize(m_CI.width * m_CI.height * 4);
			memcpy(imageData.data(), m_LocalBuffer, m_CI.width * m_CI.height * 4);

			if (m_LocalBuffer)
				stbi_image_free(m_LocalBuffer);

		}
		else
		{
			for (size_t i = 0; i < 6; i++)
			{
				m_LocalBuffer = stbi_load(m_CI.filepaths[i].c_str(), (int*)&m_CI.width, (int*)&m_CI.height, &m_BPP, 4);
				if (i == 0)
					imageData.resize(6 * m_CI.width * m_CI.height * 4);

				memcpy(imageData.data() + (i * m_CI.width * m_CI.height * 4), m_LocalBuffer, (m_CI.width * m_CI.height * 4));

				if (m_LocalBuffer)
					stbi_image_free(m_LocalBuffer);
			}

		}
		if (m_CI.depth == 0)
			m_CI.depth = 1;
	}
	else
	{
		imageData.resize(m_CI.size);
		memcpy(imageData.data(), m_CI.data, m_CI.size);
	}

	m_TextureUploadBufferCI.debugName = (std::string("GEAR_CORE_TextureUploadBuffer: ") + m_TextureName).c_str();
	m_TextureUploadBufferCI.device = m_CI.device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_TextureUploadBufferCI.size = imageData.size();
	m_TextureUploadBufferCI.imageDimension = { m_CI.width, m_CI.height, 4 };
	m_TextureUploadBufferCI.data = imageData.data();
	m_TextureUploadBufferCI.pMemoryBlock = s_MB_CPU_Upload;
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

	m_TextureCI.debugName = (std::string("GEAR_CORE_Texture: ") + m_TextureName).c_str();
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
	m_TextureCI.pMemoryBlock = s_MB_GPU_Usage;
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

	m_FinalBarrierCI.type = Barrier::Type::IMAGE;
	m_FinalBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_FinalBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_FinalBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.pImage = m_Texture;
	m_FinalBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_FinalBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_FinalBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_FinalBarrier = Barrier::Create(&m_FinalBarrierCI);

	m_TextureImageViewCI.debugName = (std::string("GEAR_CORE_TextureImageView: ") + m_TextureName).c_str();
	m_TextureImageViewCI.device = m_CI.device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, static_cast<uint32_t>(m_Cubemap ? 6 : 1) };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
	s_UID++;
}

Texture::~Texture()
{
}

void Texture::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_TextureUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize(1048576 * 1024);
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_Texture";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize(1048576 * 1024);
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void Texture::GetInitialTransition(std::vector<Ref<Barrier>>& barriers, bool force)
{
	if (!m_InitialTransition|| force)
	{
		barriers.push_back(m_InitialBarrier);
		m_InitialTransition = true;
	}
}

void Texture::Upload(Ref<CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex, bool force)
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

void Texture::GetFinalTransition(std::vector<Ref<Barrier>>& barriers, bool force)
{
	if (!m_FinalTransition || force)
	{
		barriers.push_back(m_FinalBarrier);
		m_FinalTransition = true;
	}
}

void Texture::CreateSampler()
{
	m_SamplerCI.debugName = (std::string("GEAR_CORE_Sampler: ") + m_TextureName).c_str();
	m_SamplerCI.device = m_CI.device;
	m_SamplerCI.magFilter = Sampler::Filter::LINEAR;
	m_SamplerCI.minFilter = Sampler::Filter::LINEAR;
	m_SamplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	m_SamplerCI.addressModeU = m_TileFactor > 1.0f ? Sampler::AddressMode::REPEAT : Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.addressModeV = m_TileFactor > 1.0f ? Sampler::AddressMode::REPEAT : Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.addressModeW = m_TileFactor > 1.0f ? Sampler::AddressMode::REPEAT : Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.mipLodBias = 1.0f;
	m_SamplerCI.anisotropyEnable =  m_AnisotrophicValue > 1.0f;
	m_SamplerCI.maxAnisotropy = m_AnisotrophicValue;
	m_SamplerCI.compareEnable = false;
	m_SamplerCI.compareOp = CompareOp::NEVER;
	m_SamplerCI.minLod = 0.0f;
	m_SamplerCI.maxLod = 1.0f;
	m_SamplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	m_SamplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&m_SamplerCI);
}