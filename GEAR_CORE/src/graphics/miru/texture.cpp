#include "texture.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> Texture::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> Texture::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> Texture::s_MB_GPU_Usage = nullptr;

Texture::Texture(void* device, const std::string& filepath)
	:m_Device(device), m_Filepath(filepath), m_LocalBuffer(nullptr)
{
	InitialiseMemory();

	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

	m_TextureUploadBufferCI.debugName = (std::string("GEAR_CORE_TextureUploadBuffer:") + filepath).c_str();
	m_TextureUploadBufferCI.device = m_Device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_TextureUploadBufferCI.size = m_Width * m_Height * 4;
	m_TextureUploadBufferCI.imageDimension = { (uint32_t)m_Width, (uint32_t)m_Height, 4 };
	m_TextureUploadBufferCI.data = m_LocalBuffer;
	m_TextureUploadBufferCI.pMemoryBlock = s_MB_CPU_Upload;
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

	m_TextureCI.debugName = (std::string("GEAR_CORE_Texture:") + filepath).c_str();
	m_TextureCI.device = m_Device;;
	m_TextureCI.type = Image::Type::TYPE_2D;
	m_TextureCI.format = Image::Format::R8G8B8A8_UNORM;
	m_TextureCI.width = m_Width;
	m_TextureCI.height = m_Height;
	m_TextureCI.depth = m_Depth = 1;
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = 1;
	m_TextureCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_TextureCI.usage = Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT;
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = m_Width * m_Height * 4;
	m_TextureCI.data = nullptr;
	m_TextureCI.pMemoryBlock = s_MB_GPU_Usage;
	m_Texture = Image::Create(&m_TextureCI);

	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);

	m_InitialBarrierCI.type = Barrier::Type::IMAGE;
	m_InitialBarrierCI.srcAccess = Barrier::AccessBit::NONE;
	m_InitialBarrierCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_InitialBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_InitialBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_InitialBarrierCI.pImage = m_Texture;
	m_InitialBarrierCI.oldLayout = Image::Layout::UNKNOWN;
	m_InitialBarrierCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_InitialBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_InitialBarrier = Barrier::Create(&m_InitialBarrierCI);

	m_FinalBarrierCI.type = Barrier::Type::IMAGE;
	m_FinalBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_FinalBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_FinalBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.pImage = m_Texture;
	m_FinalBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_FinalBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_FinalBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_FinalBarrier = Barrier::Create(&m_FinalBarrierCI);

	m_TextureImageViewCI.debugName = (std::string("GEAR_CORE_TextureImageView:") + filepath).c_str();
	m_TextureImageViewCI.device = m_Device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
}

Texture::Texture(void* device, const std::vector<std::string>& cubemapFilepaths)
	:m_Device(device), m_CubemapFilepaths(cubemapFilepaths), m_LocalBuffer(nullptr), m_Cubemap(true)
{	
	if (m_CubemapFilepaths.size() != 6)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::Texture: Invalid number of filepaths for cubemap." << std::endl;
		return;
	}

	std::vector<uint8_t> cubemapImageData;
	for (size_t i = 0; i < 6; i++)
	{
		m_LocalBuffer = stbi_load(m_CubemapFilepaths[i].c_str(), &m_Width, &m_Height, &m_BPP, 4);
		if (i == 0)
			cubemapImageData.resize(6 * m_Width * m_Height * 4);
		
		memcpy(cubemapImageData.data() + (i * m_Width * m_Height * 4), m_LocalBuffer, (m_Width * m_Height * 4));

		if (m_LocalBuffer)
			stbi_image_free(m_LocalBuffer);
	}

	InitialiseMemory();

	m_TextureUploadBufferCI.debugName = (std::string("GEAR_CORE_TextureUploadBuffer:") + m_CubemapFilepaths[0]).c_str();;
	m_TextureUploadBufferCI.device = m_Device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_TextureUploadBufferCI.size = cubemapImageData.size();
	m_TextureUploadBufferCI.data = cubemapImageData.data();
	m_TextureUploadBufferCI.pMemoryBlock = s_MB_CPU_Upload;
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

	m_TextureCI.debugName = (std::string("GEAR_CORE_Texture:") + m_CubemapFilepaths[0]).c_str();
	m_TextureCI.device = m_Device;
	m_TextureCI.type = Image::Type::TYPE_CUBE;
	m_TextureCI.format = Image::Format::R8G8B8A8_UNORM;
	m_TextureCI.width = m_Width;
	m_TextureCI.height = m_Height;
	m_TextureCI.depth = m_Depth = 1;
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = 6;
	m_TextureCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_TextureCI.usage = Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT;
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = cubemapImageData.size();
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
	m_InitialBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_InitialBarrier = Barrier::Create(&m_InitialBarrierCI);

	m_FinalBarrierCI.type = Barrier::Type::IMAGE;
	m_FinalBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_FinalBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_FinalBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.pImage = m_Texture;
	m_FinalBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_FinalBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_FinalBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_FinalBarrier = Barrier::Create(&m_FinalBarrierCI);

	m_TextureImageViewCI.debugName = (std::string("GEAR_CORE_TextureImageView:") + m_CubemapFilepaths[0]).c_str();
	m_TextureImageViewCI.device = m_Device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
}

Texture::Texture(void* device, unsigned char* buffer, int width, int height)
	:m_Device(device), m_LocalBuffer(nullptr), m_Width(width), m_Height(height), m_Depth(1), m_BPP(4)
{
	InitialiseMemory();
	
	m_TextureUploadBufferCI.debugName = (std::string("GEAR_CORE_TextureUploadBuffer:")).c_str();;
	m_TextureUploadBufferCI.device = m_Device;
	m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_TextureUploadBufferCI.size = m_Width * m_Height * m_BPP;
	m_TextureUploadBufferCI.data = buffer;
	m_TextureUploadBufferCI.pMemoryBlock = s_MB_CPU_Upload;
	m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);

	m_TextureCI.debugName = (std::string("GEAR_CORE_Texture:")).c_str();
	m_TextureCI.device = m_Device;
	m_TextureCI.type = Image::Type::TYPE_2D;
	m_TextureCI.format = m_BPP == 3 ? Image::Format::R8G8B8_UNORM : Image::Format::R8G8B8A8_UNORM;
	m_TextureCI.width = m_Width;
	m_TextureCI.height = m_Height;
	m_TextureCI.depth = m_Depth = 1;
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = 1;
	m_TextureCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_TextureCI.usage = Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT;
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = m_Width * m_Height * m_BPP;
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
	m_InitialBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_InitialBarrier = Barrier::Create(&m_InitialBarrierCI);

	m_FinalBarrierCI.type = Barrier::Type::IMAGE;
	m_FinalBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_FinalBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_FinalBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.pImage = m_Texture;
	m_FinalBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_FinalBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_FinalBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_FinalBarrier = Barrier::Create(&m_FinalBarrierCI);

	m_TextureImageViewCI.debugName = (std::string("GEAR_CORE_TextureImageView:")).c_str();
	m_TextureImageViewCI.device = m_Device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
}

Texture::Texture(void* device, unsigned char* buffer, Image::Type type, Image::Format format, int multisample, int width, int height, int depth)
	:m_Device(device), m_LocalBuffer(buffer), m_Width(width), m_Height(height), m_Depth(depth), m_BPP(0), m_Multisample(multisample)
{
	InitialiseMemory();
	
	if (buffer)
	{
		m_TextureUploadBufferCI.debugName = (std::string("GEAR_CORE_TextureUploadBuffer:")).c_str();;
		m_TextureUploadBufferCI.device = m_Device;
		m_TextureUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
		m_TextureUploadBufferCI.size = m_Width * m_Height * m_Depth * m_BPP;
		m_TextureUploadBufferCI.data = buffer;
		m_TextureUploadBufferCI.pMemoryBlock = s_MB_CPU_Upload;
		m_TextureUploadBuffer = Buffer::Create(&m_TextureUploadBufferCI);
	}

	m_DepthTexture = format >= Image::Format::D16_UNORM;

	m_TextureCI.debugName = (std::string("GEAR_CORE_Texture:")).c_str();
	m_TextureCI.device = m_Device;
	m_TextureCI.type = type;
	m_TextureCI.format = format;
	m_TextureCI.width = m_Width;
	m_TextureCI.height = m_Height;
	m_TextureCI.depth = m_Depth;
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = 1;
	m_TextureCI.sampleCount = (Image::SampleCountBit)multisample;
	m_TextureCI.usage = (m_DepthTexture ? Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT : Image::UsageBit::COLOUR_ATTACHMENT_BIT) | Image::UsageBit::STORAGE_BIT | Image::UsageBit::SAMPLED_BIT | Image::UsageBit::TRANSFER_DST_BIT;
	m_TextureCI.layout = Image::Layout::UNKNOWN;
	m_TextureCI.size = m_Width * m_Height * m_BPP;
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
	m_InitialBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_InitialBarrier = Barrier::Create(&m_InitialBarrierCI);

	m_FinalBarrierCI.type = Barrier::Type::IMAGE;
	m_FinalBarrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	m_FinalBarrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	m_FinalBarrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	m_FinalBarrierCI.pImage = m_Texture;
	m_FinalBarrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
	m_FinalBarrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_FinalBarrierCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_FinalBarrier = Barrier::Create(&m_FinalBarrierCI);

	m_TextureImageViewCI.debugName = (std::string("GEAR_CORE_TextureImageView:")).c_str();
	m_TextureImageViewCI.device = m_Device;
	m_TextureImageViewCI.pImage = m_Texture;
	m_TextureImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_TextureImageView = ImageView::Create(&m_TextureImageViewCI);

	CreateSampler();
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
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_TextureUsage";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void Texture::GetInitialTransition(std::vector<Ref<Barrier>>& barriers)
{ 
	barriers.push_back(m_InitialBarrier); 
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
				bic.bufferOffset = (i * m_Width * m_Height * 4);
				bic.bufferRowLength = 0;
				bic.bufferImageHeight = 0;
				bic.imageSubresource = { Image::AspectBit::COLOUR_BIT, 0, (uint32_t)i, 1 };
				bic.imageOffset = { 0, 0, 0 };
				bic.imageExtent = { (uint32_t)m_Width, (uint32_t)m_Height, (uint32_t)m_Depth };
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
			bic.imageExtent = { (uint32_t)m_Width, (uint32_t)m_Height, (uint32_t)m_Depth };
			bics.push_back(bic);
		}

		cmdBuffer->CopyBufferToImage(cmdBufferIndex, m_TextureUploadBuffer, m_Texture, Image::Layout::TRANSFER_DST_OPTIMAL, bics);
		m_Upload = true;
	}
}

void Texture::GetFinalTransition(std::vector<Ref<Barrier>>& barriers) 
{
	barriers.push_back(m_FinalBarrier); 
}

void Texture::CreateSampler()
{
	m_SamplerCI.debugName = (std::string("GEAR_CORE_Sampler:") + m_Filepath).c_str();
	m_SamplerCI.device = m_Device;
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
	m_SamplerCI.unnormalisedCoordinates = false; //m_TileFactor > 1.0f;
	m_Sampler = Sampler::Create(&m_SamplerCI);
}