#include "gear_core_common.h"
#include "PostProcessing.h"

#include "AllocatorManager.h"
#include "RenderPipeline.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "UniformBufferStructures.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Ref<RenderPipeline> PostProcessing::s_BloomPreFilter;
Ref<RenderPipeline> PostProcessing::s_BloomDownsample;
Ref<RenderPipeline> PostProcessing::s_BloomUpsample;

std::array<std::vector<Ref<ImageView>>, 2>		PostProcessing::s_ImageViews;
std::array<std::vector<Ref<DescriptorSet>>, 2>	PostProcessing::s_DescSets;

typedef UniformBufferStructures::BloomInfo BloomInfoUB;
static std::array<Ref<Uniformbuffer<BloomInfoUB>>, 2> m_BloomInfoUBs;

static std::array<Ref<Sampler>, 2> s_Samplers;

PostProcessing::PostProcessing()
{
}

PostProcessing::~PostProcessing()
{
}

void PostProcessing::Bloom(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI)
{
	BloomPreFilter(cmdBuffer, frameIndex, IRI);
	
	ImageResourceInfo IRI_output;
	IRI_output.image = s_ImageViews[frameIndex].back()->GetCreateInfo().pImage;
	IRI_output.oldLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	IRI_output.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
	IRI_output.srcStage = PipelineStageBit::COMPUTE_SHADER_BIT;
	BloomDownsample(cmdBuffer, frameIndex, IRI_output);
	
	BloomUpsample(cmdBuffer, frameIndex, IRI);
}

void PostProcessing::BloomPreFilter(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI)
{
	if (!s_BloomPreFilter)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/BloomPrefilter.grpf";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_BloomPreFilter = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();
	uint32_t minSize = std::min(IRI.image->GetCreateInfo().width, IRI.image->GetCreateInfo().height);
	uint32_t levels = static_cast<uint32_t>(log2(static_cast<double>(minSize / 8)));

	//Input
	ImageView::CreateInfo inputImageViewCI;
	inputImageViewCI.debugName = "GEAR_CORE_ImageView_Bloom_Prefilter_Input";
	inputImageViewCI.device = device;
	inputImageViewCI.pImage = IRI.image;
	inputImageViewCI.viewType = Image::Type::TYPE_2D;
	inputImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	Ref<ImageView> inputImageView = ImageView::Create(&inputImageViewCI);

	Image::CreateInfo outputImageCI = IRI.image->GetCreateInfo();
	outputImageCI.debugName = "GEAR_CORE_Image_Bloom_Prefilter_Output";
	outputImageCI.mipLevels = levels;
	outputImageCI.usage |= Image::UsageBit::SAMPLED_BIT;
	Ref<Image> outputImage = Image::Create(&outputImageCI);
	
	//Output
	ImageView::CreateInfo outputImageViewCI;
	outputImageViewCI.debugName = "GEAR_CORE_ImageView_Bloom_Prefilter_Output";
	outputImageViewCI.device = device;
	outputImageViewCI.pImage = outputImage;
	outputImageViewCI.viewType = Image::Type::TYPE_2D;
	outputImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	Ref<ImageView> outputImageView = ImageView::Create(&outputImageViewCI);
	
	float zero[sizeof(BloomInfoUB)] = { 0 };
	Uniformbuffer<BloomInfoUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Buffer_BloomInfoUB";
	ubCI.device = device;
	ubCI.data = zero;
	m_BloomInfoUBs[frameIndex] = CreateRef<Uniformbuffer<BloomInfoUB>>(&ubCI);
	m_BloomInfoUBs[frameIndex]->threshold = 3.0f;
	m_BloomInfoUBs[frameIndex]->upsampleScale = 2.0f;
	m_BloomInfoUBs[frameIndex]->SubmitData();

	//Descriptor Pool and Set
	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Bloom_Prefilter";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::STORAGE_IMAGE, 2 }, {DescriptorType::UNIFORM_BUFFER, 1 } };
	m_DescPoolCI.maxSets = 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_Bloom_Prefilter";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_BloomPreFilter->GetDescriptorSetLayouts();
	Ref<DescriptorSet> m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	m_DescSet->AddImage(0, 0, { { nullptr, inputImageView, Image::Layout::GENERAL } });
	m_DescSet->AddImage(0, 1, { { nullptr, outputImageView, Image::Layout::GENERAL} });
	m_DescSet->AddBuffer(0, 2, { { m_BloomInfoUBs[frameIndex]->GetBufferView() } });
	m_DescSet->Update();

	SaveImageViewsAndDescriptorSets({ inputImageView, outputImageView }, { m_DescSet }, frameIndex);

	//Record Compute CommandBuffer
	{
		m_BloomInfoUBs[frameIndex]->Upload(cmdBuffer, frameIndex, true);

		std::vector<Ref<Barrier>> barriers;
		barriers.clear();

		Barrier::CreateInfo barrierCI;
		barrierCI.type = Barrier::Type::IMAGE;
		barrierCI.srcAccess = IRI.srcAccess;
		barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.pImage = IRI.image;
		barrierCI.oldLayout = IRI.oldLayout;
		barrierCI.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS: Image::Layout::GENERAL;
		barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		barriers.emplace_back(Barrier::Create(&barrierCI));
		barrierCI.type = Barrier::Type::IMAGE;
		barrierCI.srcAccess = Barrier::AccessBit(0);
		barrierCI.dstAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.pImage = outputImage;
		barrierCI.oldLayout = outputImageCI.layout;
		barrierCI.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
		barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, levels, 0, 1 };
		barriers.emplace_back(Barrier::Create(&barrierCI));
		if (GraphicsAPI::IsD3D12())
		{
			barrierCI.type = Barrier::Type::BUFFER;
			barrierCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
			barrierCI.dstAccess = Barrier::AccessBit::UNIFORM_READ_BIT;
			barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.pBuffer = m_BloomInfoUBs[frameIndex]->GetBuffer();
			barrierCI.offset = 0;
			barrierCI.size = m_BloomInfoUBs[frameIndex]->GetSize();
			barriers.emplace_back(Barrier::Create(&barrierCI));
		}

		cmdBuffer->PipelineBarrier(frameIndex, IRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindPipeline(frameIndex, s_BloomPreFilter->GetPipeline());
		cmdBuffer->BindDescriptorSets(frameIndex, { m_DescSet }, s_BloomPreFilter->GetPipeline());
		uint32_t width = std::max(IRI.image->GetCreateInfo().width / 8, uint32_t(1));
		uint32_t height = std::max(IRI.image->GetCreateInfo().height / 8, uint32_t(1));
		uint32_t depth = 1;
		cmdBuffer->Dispatch(frameIndex, width, height, depth);
	}
}

void PostProcessing::BloomDownsample(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI)
{
	if (!s_BloomDownsample)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/BloomDownsample.grpf";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_BloomDownsample = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();
	const uint32_t& levels = IRI.image->GetCreateInfo().mipLevels;

	//Input And Output ImageViews
	std::vector<Ref<ImageView>> m_ImageViews;
	m_ImageViews.reserve(levels);

	ImageView::CreateInfo outputImageViewCI;
	outputImageViewCI.debugName = "GEAR_CORE_ImageView_Bloom_Downsample_Output";
	outputImageViewCI.device = device;
	outputImageViewCI.pImage = IRI.image;
	outputImageViewCI.viewType = Image::Type::TYPE_2D;
	for (uint32_t i = 0; i < levels; i++)
	{
		outputImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 };
		m_ImageViews.emplace_back(ImageView::Create(&outputImageViewCI));
	}

	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "GEAR_CORE_Sampler_Bloom_Downsample_Output";;
	samplerCI.device = device;
	samplerCI.magFilter = Sampler::Filter::LINEAR;
	samplerCI.minFilter = Sampler::Filter::LINEAR;
	samplerCI.mipmapMode = Sampler::MipmapMode::NEAREST;
	samplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.anisotropyEnable = false;
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 0.0f;
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_TRANSPARENT_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	s_Samplers[frameIndex] = Sampler::Create(&samplerCI);

	//Descriptor Pool and Set
	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Bloom_Downsample";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::SAMPLED_IMAGE, (levels - 1)}, {DescriptorType::STORAGE_IMAGE, (levels - 1)} };
	m_DescPoolCI.maxSets = levels - 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	std::vector<Ref<DescriptorSet>> m_DescSets;
	m_DescSets.reserve(levels);

	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_Bloom_Downsample";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_BloomDownsample->GetDescriptorSetLayouts();
	Image::Layout m_ImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { s_Samplers[frameIndex],	m_ImageViews[i + 0], Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		m_DescSets[i]->AddImage(0, 1, { { nullptr,					m_ImageViews[i + 1], m_ImageLayout } });
		m_DescSets[i]->Update();
	}

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(m_ImageViews, m_DescSets, frameIndex);

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		cmdBuffer->BindPipeline(frameIndex, s_BloomDownsample->GetPipeline());
		for (uint32_t i = 1; i < levels; i++)
		{
			Barrier::CreateInfo barrierCI;
			barrierCI.type = Barrier::Type::IMAGE;
			barrierCI.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.pImage = IRI.image;
			barrierCI.oldLayout = m_ImageLayout;
			barrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i-1, 1, 0, 1 };
			
			barriers.clear();
			barriers.emplace_back(Barrier::Create(&barrierCI));
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

			cmdBuffer->BindDescriptorSets(frameIndex, { m_DescSets[i - 1] }, s_BloomDownsample->GetPipeline());
			uint32_t width = 1 + std::max((IRI.image->GetCreateInfo().width >> i) / 8, uint32_t(1));	//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			uint32_t height = 1 + std::max((IRI.image->GetCreateInfo().height >> i) / 8, uint32_t(1));	//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			uint32_t depth = 1;
			cmdBuffer->Dispatch(frameIndex, width, height, depth);

			barrierCI.type = Barrier::Type::IMAGE;
			barrierCI.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.pImage = IRI.image;
			barrierCI.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			barrierCI.newLayout = m_ImageLayout;
			barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i-1, 1, 0, 1 };

			barriers.clear();
			barriers.emplace_back(Barrier::Create(&barrierCI));
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}
	}
}

void PostProcessing::BloomUpsample(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI)
{
	if (!s_BloomUpsample)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/BloomUpsample.grpf";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_BloomUpsample = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();
	const uint32_t& levels = s_ImageViews[frameIndex][1]->GetCreateInfo().pImage->GetCreateInfo().mipLevels;

	std::vector<Ref<ImageView>> m_ImageViews;
	m_ImageViews.resize(levels);
	std::copy(s_ImageViews[frameIndex].begin() + 2, s_ImageViews[frameIndex].end(), m_ImageViews.begin());
	m_ImageViews[0] = s_ImageViews[frameIndex][0];

	//Descriptor Pool and Set
	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Bloom_Upsample";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::SAMPLED_IMAGE, (levels - 1)}, {DescriptorType::STORAGE_IMAGE, (levels - 1)}, {DescriptorType::UNIFORM_BUFFER, (levels - 1)} };
	m_DescPoolCI.maxSets = levels - 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	std::vector<Ref<DescriptorSet>> m_DescSets;
	m_DescSets.reserve(levels);

	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_Bloom_Upsample";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_BloomUpsample->GetDescriptorSetLayouts();
	Image::Layout m_ImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { s_Samplers[frameIndex],	m_ImageViews[i + 1], Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		m_DescSets[i]->AddImage(0, 1, { { nullptr,					m_ImageViews[i + 0], m_ImageLayout } });
		m_DescSets[i]->AddBuffer(0, 2, { { m_BloomInfoUBs[frameIndex]->GetBufferView() } });
		m_DescSets[i]->Update();
	}

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets({}, m_DescSets, frameIndex);
	
	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;
		Barrier::CreateInfo barrierCI;

		cmdBuffer->BindPipeline(frameIndex, s_BloomUpsample->GetPipeline());
		for (uint32_t i = levels - 1; i >= 1; i--)
		{
			barrierCI.type = Barrier::Type::IMAGE;
			barrierCI.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.pImage = m_ImageViews[i]->GetCreateInfo().pImage;
			barrierCI.oldLayout = m_ImageLayout;
			barrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 };

			barriers.clear();
			barriers.emplace_back(Barrier::Create(&barrierCI));
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

			cmdBuffer->BindDescriptorSets(frameIndex, { m_DescSets[i - 1] }, s_BloomUpsample->GetPipeline());
			uint32_t width = 1 + std::max((IRI.image->GetCreateInfo().width >> (i - 1)) / 8, uint32_t(1));	//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			uint32_t height = 1 + std::max((IRI.image->GetCreateInfo().height >> (i - 1)) / 8, uint32_t(1));	//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			uint32_t depth = 1;
			cmdBuffer->Dispatch(frameIndex, width, height, depth);

			barrierCI.type = Barrier::Type::IMAGE;
			barrierCI.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			barrierCI.pImage = m_ImageViews[i]->GetCreateInfo().pImage;
			barrierCI.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			barrierCI.newLayout = m_ImageLayout;
			barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 };

			barriers.clear();
			barriers.emplace_back(Barrier::Create(&barrierCI));
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		barrierCI.type = Barrier::Type::IMAGE;
		barrierCI.srcAccess = Barrier::AccessBit::SHADER_READ_BIT;
		barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		barrierCI.pImage = IRI.image;
		barrierCI.oldLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
		barrierCI.newLayout = IRI.oldLayout;
		barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		barriers.clear();
		barriers.emplace_back(Barrier::Create(&barrierCI));
		cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
	}
}
