#include "gear_core_common.h"
#include "GenerateMipMaps.h"
#include "Graphics/MemoryBlockManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

gear::Ref<RenderPipeline> GenerateMipMaps::s_GenerateMipMapPipeline;
RenderPipeline::LoadInfo GenerateMipMaps::s_GenerateMipMapPipelineLI;

GenerateMipMaps::GenerateMipMaps(gear::Ref<Texture>& texture)
{
	if(!s_GenerateMipMapPipeline)
	{ 
		s_GenerateMipMapPipelineLI.filepath = "res/pipelines/GenerateMipMaps.grpf.json";
		s_GenerateMipMapPipelineLI.viewportWidth = 0.0f;
		s_GenerateMipMapPipelineLI.viewportHeight = 0.0f;
		s_GenerateMipMapPipelineLI.renderPass = nullptr;
		s_GenerateMipMapPipelineLI.subpassIndex = 0;
		s_GenerateMipMapPipeline = CreateRef<RenderPipeline>(&s_GenerateMipMapPipelineLI);
	}

	m_ComputeCmdPoolCI.debugName = "GEAR_Core_CommandPool_GenerateMipMaps_Compute";
	m_ComputeCmdPoolCI.pContext = MemoryBlockManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueFamilyIndex = 1;
	m_ComputeCmdPool = CommandPool::Create(&m_ComputeCmdPoolCI);

	m_ComputeCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_GenerateMipMaps_Compute";
	m_ComputeCmdBufferCI.pCommandPool = m_ComputeCmdPool;
	m_ComputeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_ComputeCmdBufferCI.commandBufferCount = 1;
	m_ComputeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	m_ComputeCmdBuffer = CommandBuffer::Create(&m_ComputeCmdBufferCI);

	const uint32_t& levels = texture->GetCreateInfo().mipLevels;

	m_ImageViews.reserve(levels);
	m_ImageViewCI.debugName = "GEAR_Core_ImageView_GenerateMipMaps";
	m_ImageViewCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_ImageViewCI.pImage = texture->GetTexture();
	for (uint32_t i = 0; i < levels; i++)
	{
		m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 };
		m_ImageViews.emplace_back(ImageView::Create(&m_ImageViewCI));
	}

	m_SamplerCI.debugName = "GEAR_CORE_Sampler_GenerateMipMaps";
	m_SamplerCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_SamplerCI.magFilter = Sampler::Filter::LINEAR;
	m_SamplerCI.minFilter = Sampler::Filter::LINEAR;;
	m_SamplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	m_SamplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
	m_SamplerCI.mipLodBias = 1.0f;
	m_SamplerCI.anisotropyEnable = false;
	m_SamplerCI.maxAnisotropy = 1.0f;
	m_SamplerCI.compareEnable = false;
	m_SamplerCI.compareOp = CompareOp::NEVER;
	m_SamplerCI.minLod = 0.0f;
	m_SamplerCI.maxLod = 1.0f;
	m_SamplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	m_SamplerCI.unnormalisedCoordinates = false;
	m_Sampler = Sampler::Create(&m_SamplerCI);

	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_GenerateMipMaps";
	m_DescPoolCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::STORAGE_IMAGE, (levels - 1)}, {DescriptorType::COMBINED_IMAGE_SAMPLER, (levels - 1)} };
	m_DescPoolCI.maxSets = levels - 1;
	m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	m_DescSets.reserve(levels);
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_GenerateMipMaps";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_GenerateMipMapPipeline->GetDescriptorSetLayouts();
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { m_Sampler, nullptr, Image::Layout::UNKNOWN} });
		m_DescSets[i]->AddImage(0, 1, { { nullptr, m_ImageViews[i + 0], Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		m_DescSets[i]->AddImage(0, 2, { { nullptr, m_ImageViews[i + 1], Image::Layout::GENERAL } });
		m_DescSets[i]->Update();
	}

}

GenerateMipMaps::~GenerateMipMaps()
{
}
