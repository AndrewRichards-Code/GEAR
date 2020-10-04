#include "gear_core_common.h"
#include "ImageProcessing.h"

#include "MemoryBlockManager.h"
#include "RenderPipeline.h"
#include "Texture.h"
#include "directx12/D3D12CommandPoolBuffer.h"
#include "directx12/D3D12Image.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

gear::Ref<RenderPipeline> ImageProcessing::s_PipelineMipMap;
gear::Ref<RenderPipeline> ImageProcessing::s_PipelineMipMapArray;
gear::Ref<RenderPipeline> ImageProcessing::s_PipelineEquirectangularToCube;

ImageProcessing::ImageProcessing()
{
}

ImageProcessing::~ImageProcessing()
{
}

void ImageProcessing::GenerateMipMaps(Texture* texture)
{
	if(!s_PipelineMipMap)
	{ 
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = MemoryBlockManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/mipmap.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineMipMap = CreateRef<RenderPipeline>(&s_PipelineLI);
	}
	if (!s_PipelineMipMapArray)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = MemoryBlockManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/mipmapArray.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineMipMapArray = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	CommandPool::CreateInfo m_ComputeCmdPoolCI;
	m_ComputeCmdPoolCI.debugName = "GEAR_CORE_CommandPool_MipMap_Compute";
	m_ComputeCmdPoolCI.pContext = MemoryBlockManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueFamilyIndex = 1;
	miru::Ref<CommandPool> m_ComputeCmdPool = CommandPool::Create(&m_ComputeCmdPoolCI);

	CommandBuffer::CreateInfo m_ComputeCmdBufferCI;
	m_ComputeCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_MipMap_Compute";
	m_ComputeCmdBufferCI.pCommandPool = m_ComputeCmdPool;
	m_ComputeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_ComputeCmdBufferCI.commandBufferCount = 1;
	m_ComputeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	miru::Ref<CommandBuffer> m_ComputeCmdBuffer = CommandBuffer::Create(&m_ComputeCmdBufferCI);

	const uint32_t& levels = texture->GetCreateInfo().mipLevels;
	const uint32_t& layers = texture->GetCreateInfo().arrayLayers;

	miru::Ref<RenderPipeline>& pipeline = s_PipelineMipMap;
	if (layers > 1)
		pipeline = s_PipelineMipMapArray;

	std::vector<miru::Ref<ImageView>> m_ImageViews;
	m_ImageViews.reserve(levels);

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_MipMap";
	m_ImageViewCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_ImageViewCI.pImage = texture->GetTexture();
	m_ImageViewCI.viewType = texture->GetCreateInfo().type;
	for (uint32_t i = 0; i < levels; i++)
	{
		m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
		m_ImageViews.emplace_back(ImageView::Create(&m_ImageViewCI));
	}

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_MipMap";
	m_DescPoolCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::SAMPLED_IMAGE, (levels - 1)}, {DescriptorType::STORAGE_IMAGE, (levels - 1)} };
	m_DescPoolCI.maxSets = levels - 1;
	miru::Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	std::vector<miru::Ref<DescriptorSet>> m_DescSets;
	m_DescSets.reserve(levels);

	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_MipMap";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = pipeline->GetDescriptorSetLayouts();
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { nullptr, m_ImageViews[i + 0], Image::Layout::GENERAL } });
		m_DescSets[i]->AddImage(0, 1, { { nullptr, m_ImageViews[i + 1], Image::Layout::GENERAL } });
		m_DescSets[i]->Update();
	}

	miru::crossplatform::Fence::CreateInfo m_FenceCI;
	m_FenceCI.debugName = "GEAR_CORE_Fence_MipMap";
	m_FenceCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_FenceCI.signaled = false;
	m_FenceCI.timeout = UINT64_MAX;
	miru::Ref<Fence> m_Fence = Fence::Create(&m_FenceCI);

	//Record Compute CommandBuffer
	{
		m_ComputeCmdBuffer->Reset(0, false);
		m_ComputeCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo toGeneral;
		toGeneral.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		toGeneral.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		toGeneral.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		toGeneral.newLayout = Image::Layout::GENERAL;
		toGeneral.subresoureRange = {};
		toGeneral.allSubresources = true;
		
		barriers.clear();
		texture->TransitionSubResources(barriers, { toGeneral });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindPipeline(0, pipeline->GetPipeline());
		for (uint32_t i = 1; i < levels; i++)
		{
			Texture::SubresouresTransitionInfo preDispatch;
			preDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.oldLayout = Image::Layout::GENERAL;
			preDispatch.newLayout = Image::Layout::GENERAL;
			preDispatch.subresoureRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			preDispatch.allSubresources = false;

			barriers.clear();
			texture->TransitionSubResources(barriers, { preDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

			m_ComputeCmdBuffer->BindDescriptorSets(0, { m_DescSets[i - 1] }, pipeline->GetPipeline());
			uint32_t width = std::max((texture->GetCreateInfo().width >> i) / 8, uint32_t(1));
			uint32_t height = std::max((texture->GetCreateInfo().height >> i) / 8, uint32_t(1));
			m_ComputeCmdBuffer->Dispatch(0, width, height, 1);

			Texture::SubresouresTransitionInfo postDispatch;
			postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			postDispatch.oldLayout = Image::Layout::GENERAL;
			postDispatch.newLayout = Image::Layout::GENERAL;
			postDispatch.subresoureRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			postDispatch.allSubresources = false;

			barriers.clear();
			texture->TransitionSubResources(barriers, { postDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}
	
		Texture::SubresouresTransitionInfo toTransferDst;
		toTransferDst.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		toTransferDst.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		toTransferDst.oldLayout = Image::Layout::GENERAL;
		toTransferDst.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		toTransferDst.subresoureRange = {};
		toTransferDst.allSubresources = true;

		barriers.clear();
		texture->TransitionSubResources(barriers, { toTransferDst });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->End(0);
	}

	m_ComputeCmdBuffer->Submit({ 0 }, {}, {}, {}, m_Fence);
	m_Fence->Wait();
}

void ImageProcessing::EquirectangularToCube(gear::Ref<Texture>& cubemap, gear::Ref<Texture>& texture)
{
	if (!s_PipelineEquirectangularToCube)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = MemoryBlockManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/equirectangularToCube.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineEquirectangularToCube = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	CommandPool::CreateInfo m_ComputeCmdPoolCI;
	m_ComputeCmdPoolCI.debugName = "GEAR_CORE_CommandPool_EquirectangularToCube_Compute";
	m_ComputeCmdPoolCI.pContext = MemoryBlockManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueFamilyIndex = 1;
	miru::Ref<CommandPool> m_ComputeCmdPool = CommandPool::Create(&m_ComputeCmdPoolCI);

	CommandBuffer::CreateInfo m_ComputeCmdBufferCI;
	m_ComputeCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_EquirectangularToCube_Compute";
	m_ComputeCmdBufferCI.pCommandPool = m_ComputeCmdPool;
	m_ComputeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_ComputeCmdBufferCI.commandBufferCount = 1;
	m_ComputeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	miru::Ref<CommandBuffer> m_ComputeCmdBuffer = CommandBuffer::Create(&m_ComputeCmdBufferCI);

	miru::Ref<ImageView> m_EquirectangularImageView;
	miru::Ref<ImageView> m_CubeImageView;

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Equirectangular";
	m_ImageViewCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_ImageViewCI.pImage = texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_EquirectangularImageView= ImageView::Create(&m_ImageViewCI);

	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cube";
	m_ImageViewCI.pImage = cubemap->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_CubeImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_MipMap";
	m_DescPoolCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1 }, {DescriptorType::STORAGE_IMAGE, 1 } };
	m_DescPoolCI.maxSets = 1;
	miru::Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	miru::Ref<DescriptorSet> m_DescSet;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_EquirectangularToCube";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineEquirectangularToCube->GetDescriptorSetLayouts();
	m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	m_DescSet->AddImage(0, 0, { { texture->GetTextureSampler(), m_EquirectangularImageView, Image::Layout::GENERAL } });
	m_DescSet->AddImage(0, 1, { { nullptr, m_CubeImageView, Image::Layout::GENERAL } });
	m_DescSet->Update();

	Fence::CreateInfo m_FenceCI;
	m_FenceCI.debugName = "GEAR_CORE_Fence_EquirectangularToCube";
	m_FenceCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_FenceCI.signaled = false;
	m_FenceCI.timeout = UINT64_MAX;
	miru::Ref<Fence> m_Fence = Fence::Create(&m_FenceCI);

	//Record Compute CommandBuffer
	{
		m_ComputeCmdBuffer->Reset(0, false);
		m_ComputeCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo toGeneral;
		toGeneral.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		toGeneral.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		toGeneral.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		toGeneral.newLayout = Image::Layout::GENERAL;
		toGeneral.subresoureRange = {};
		toGeneral.allSubresources = true;

		barriers.clear();
		texture->TransitionSubResources(barriers, { toGeneral });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindPipeline(0, s_PipelineEquirectangularToCube->GetPipeline());
		Texture::SubresouresTransitionInfo preDispatch;
		preDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		preDispatch.oldLayout = Image::Layout::GENERAL;
		preDispatch.newLayout = Image::Layout::GENERAL;
		preDispatch.subresoureRange = {};
		preDispatch.allSubresources = true;

		barriers.clear();
		cubemap->TransitionSubResources(barriers, { preDispatch });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineEquirectangularToCube->GetPipeline());
		uint32_t width = std::max(texture->GetCreateInfo().width / 32, uint32_t(1));
		uint32_t height = std::max(texture->GetCreateInfo().height / 32, uint32_t(1));
		uint32_t depth = 6;
		m_ComputeCmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo postDispatch;
		postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		postDispatch.oldLayout = Image::Layout::GENERAL;
		postDispatch.newLayout = Image::Layout::GENERAL;
		postDispatch.subresoureRange = {};
		postDispatch.allSubresources = true;

		barriers.clear();
		cubemap->TransitionSubResources(barriers, { postDispatch });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);


		Texture::SubresouresTransitionInfo toTransferDst;
		toTransferDst.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		toTransferDst.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		toTransferDst.oldLayout = Image::Layout::GENERAL;
		toTransferDst.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		toTransferDst.subresoureRange = {};
		toTransferDst.allSubresources = true;

		barriers.clear();
		texture->TransitionSubResources(barriers, { toTransferDst });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->End(0);
	}

	m_ComputeCmdBuffer->Submit({ 0 }, {}, {}, {}, m_Fence);
	m_Fence->Wait();
}