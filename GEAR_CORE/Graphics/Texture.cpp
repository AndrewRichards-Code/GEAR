#include "gear_core_common.h"
#include "Graphics/Texture.h"
#include "Graphics/AllocatorManager.h"

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

	//Calculate Mipmap details
	if (!mars::Utility::IsPowerOf2(m_CI.width) && !mars::Utility::IsPowerOf2(m_CI.height))
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

	//Upload buffer
	Buffer::CreateInfo imageUploadBufferCI;
	imageUploadBufferCI.debugName = "GEAR_CORE_TextureUploadBuffer: " + m_CI.debugName;
	imageUploadBufferCI.device = m_CI.device;
	imageUploadBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::TRANSFER_DST_BIT;
	imageUploadBufferCI.imageDimension = { m_CI.width, m_CI.height, std::max(m_CI.depth, m_CI.arrayLayers), Image::GetFormatSize(m_CI.format) };
	imageUploadBufferCI.size = std::max(m_CI.imageData.size(), imageUploadBufferCI.imageDimension.GetTotalSize<size_t>());
	imageUploadBufferCI.data = m_CI.imageData.data();
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
	imageCI.width = m_CI.width;
	imageCI.height = m_CI.height;
	imageCI.depth = m_CI.depth;
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
	SubmitImageData(m_CI.imageData);

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

void Texture::SetSampler(Sampler::CreateInfo* pSamplerCreateInfo)
{
	SetSampler(Sampler::Create(pSamplerCreateInfo));
}

void Texture::SetSampler(SamplerRef sampler)
{
	m_Sampler = sampler;
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
	samplerCI.anisotropyEnable = false;
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(m_CI.mipLevels);
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&samplerCI);
}
