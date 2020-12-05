#include "gear_core_common.h"
#include "ImageProcessing.h"

#include "AllocatorManager.h"
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
gear::Ref<RenderPipeline> ImageProcessing::s_PipelineDiffuseIrradiance;

ImageProcessing::ImageProcessing()
{
}

ImageProcessing::~ImageProcessing()
{
}

void ImageProcessing::GenerateMipMaps(const TextureResourceInfo& TRI)
{
	if (!TRI.texture->m_GenerateMipMaps || TRI.texture->m_Generated)
		return;

	if (!TRI.texture->IsUploaded())
	{
		GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_STATE, "Texture data has not been uploaded. Can not generate mipmaps.");
		return;
	}

	if(!s_PipelineMipMap)
	{ 
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/Mipmap.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineMipMap = CreateRef<RenderPipeline>(&s_PipelineLI);
	}
	if (!s_PipelineMipMapArray)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/MipmapArray.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineMipMapArray = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	CommandPool::CreateInfo m_ComputeCmdPoolCI;
	m_ComputeCmdPoolCI.debugName = "GEAR_CORE_CommandPool_MipMap_Compute";
	m_ComputeCmdPoolCI.pContext = AllocatorManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueType = CommandPool::QueueType::COMPUTE;
	miru::Ref<CommandPool> m_ComputeCmdPool = CommandPool::Create(&m_ComputeCmdPoolCI);

	CommandBuffer::CreateInfo m_ComputeCmdBufferCI;
	m_ComputeCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_MipMap_Compute";
	m_ComputeCmdBufferCI.pCommandPool = m_ComputeCmdPool;
	m_ComputeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_ComputeCmdBufferCI.commandBufferCount = 1;
	m_ComputeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	miru::Ref<CommandBuffer> m_ComputeCmdBuffer = CommandBuffer::Create(&m_ComputeCmdBufferCI);

	const uint32_t& levels = TRI.texture->GetCreateInfo().mipLevels;
	const uint32_t& layers = TRI.texture->GetCreateInfo().arrayLayers;

	miru::Ref<RenderPipeline>& pipeline = s_PipelineMipMap;
	if (layers > 1)
		pipeline = s_PipelineMipMapArray;

	std::vector<miru::Ref<ImageView>> m_ImageViews;
	m_ImageViews.reserve(levels);

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_MipMap";
	m_ImageViewCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_ImageViewCI.pImage = TRI.texture->GetTexture();
	m_ImageViewCI.viewType = TRI.texture->GetCreateInfo().type;
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
	Image::Layout m_ImageLayout = Image::Layout::GENERAL;
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { nullptr, m_ImageViews[i + 0], m_ImageLayout } });
		m_DescSets[i]->AddImage(0, 1, { { nullptr, m_ImageViews[i + 1], m_ImageLayout } });
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
		toGeneral.srcAccess = TRI.srcAccess;
		toGeneral.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		toGeneral.oldLayout = TRI.oldLayout;
		toGeneral.newLayout = m_ImageLayout;
		toGeneral.subresoureRange = {};
		toGeneral.allSubresources = true;
		
		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { toGeneral });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindPipeline(0, pipeline->GetPipeline());
		for (uint32_t i = 1; i < levels; i++)
		{
			Texture::SubresouresTransitionInfo preDispatch;
			preDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.oldLayout = m_ImageLayout;
			preDispatch.newLayout = m_ImageLayout;
			preDispatch.subresoureRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			preDispatch.allSubresources = false;

			barriers.clear();
			TRI.texture->TransitionSubResources(barriers, { preDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

			m_ComputeCmdBuffer->BindDescriptorSets(0, { m_DescSets[i - 1] }, pipeline->GetPipeline());
			uint32_t width = std::max((TRI.texture->GetWidth() >> i) / 8, uint32_t(1));
			uint32_t height = std::max((TRI.texture->GetHeight() >> i) / 8, uint32_t(1));
			m_ComputeCmdBuffer->Dispatch(0, width, height, 1);

			Texture::SubresouresTransitionInfo postDispatch;
			postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			postDispatch.oldLayout = m_ImageLayout;
			postDispatch.newLayout = m_ImageLayout;
			postDispatch.subresoureRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			postDispatch.allSubresources = false;

			barriers.clear();
			TRI.texture->TransitionSubResources(barriers, { postDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}
	
		Texture::SubresouresTransitionInfo toTransferDst;
		toTransferDst.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		toTransferDst.dstAccess = TRI.dstAccess;
		toTransferDst.oldLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : m_ImageLayout; //D3D12: D3D12_RESOURCE_STATE_COMMON is promoted to D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE after Dispatch().
		toTransferDst.newLayout = TRI.newLayout;
		toTransferDst.subresoureRange = {};
		toTransferDst.allSubresources = true;

		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { toTransferDst });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->End(0);
	}

	m_ComputeCmdBuffer->Submit({ 0 }, {}, {}, {}, m_Fence);
	m_Fence->Wait();
	
	TRI.texture->m_Generated = true;
}

void ImageProcessing::EquirectangularToCube(const TextureResourceInfo& cubeTRI, const TextureResourceInfo& equirectangularTRI)
{
	if (!s_PipelineEquirectangularToCube)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/EquirectangularToCube.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineEquirectangularToCube = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	CommandPool::CreateInfo m_ComputeCmdPoolCI;
	m_ComputeCmdPoolCI.debugName = "GEAR_CORE_CommandPool_EquirectangularToCube_Compute";
	m_ComputeCmdPoolCI.pContext = AllocatorManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueType = CommandPool::QueueType::COMPUTE;
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
	m_ImageViewCI.pImage = equirectangularTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_EquirectangularImageView= ImageView::Create(&m_ImageViewCI);

	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cube";
	m_ImageViewCI.pImage = cubeTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_CubeImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_EquirectangularToCube";
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
	Image::Layout m_EquirectangularImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	Image::Layout m_CubeImageLayout = Image::Layout::GENERAL;
	m_DescSet->AddImage(0, 0, { { equirectangularTRI.texture->GetTextureSampler(), m_EquirectangularImageView, m_EquirectangularImageLayout } });
	m_DescSet->AddImage(0, 1, { { nullptr, m_CubeImageView, m_CubeImageLayout } });
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

		Texture::SubresouresTransitionInfo equirectangularPreDispatch;
		equirectangularPreDispatch.srcAccess = equirectangularTRI.srcAccess;
		equirectangularPreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		equirectangularPreDispatch.oldLayout = equirectangularTRI.oldLayout;
		equirectangularPreDispatch.newLayout = m_EquirectangularImageLayout;
		equirectangularPreDispatch.subresoureRange = {};
		equirectangularPreDispatch.allSubresources = true;

		if (equirectangularPreDispatch.oldLayout != equirectangularPreDispatch.newLayout)
		{
			barriers.clear();
			equirectangularTRI.texture->TransitionSubResources(barriers, { equirectangularPreDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		m_ComputeCmdBuffer->BindPipeline(0, s_PipelineEquirectangularToCube->GetPipeline());
		Texture::SubresouresTransitionInfo cubePreDispatch;
		cubePreDispatch.srcAccess = cubeTRI.srcAccess;
		cubePreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		cubePreDispatch.oldLayout = cubeTRI.oldLayout;
		cubePreDispatch.newLayout = m_CubeImageLayout;
		cubePreDispatch.subresoureRange = {};
		cubePreDispatch.allSubresources = true;

		if (cubePreDispatch.oldLayout != cubePreDispatch.newLayout)
		{
			barriers.clear();
			cubeTRI.texture->TransitionSubResources(barriers, { cubePreDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		m_ComputeCmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineEquirectangularToCube->GetPipeline());
		uint32_t width = std::max(cubeTRI.texture->GetWidth() / 32, uint32_t(1));
		uint32_t height = std::max(cubeTRI.texture->GetHeight() / 32, uint32_t(1));
		uint32_t depth = 6;
		m_ComputeCmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo cubePostDispatch;
		cubePostDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		cubePostDispatch.dstAccess = cubeTRI.dstAccess;
		cubePostDispatch.oldLayout = cubePreDispatch.newLayout;
		cubePostDispatch.newLayout = cubeTRI.newLayout;
		cubePostDispatch.subresoureRange = {};
		cubePostDispatch.allSubresources = true;
		
		if (cubePostDispatch.oldLayout != cubePostDispatch.newLayout)
		{
			barriers.clear();
			cubeTRI.texture->TransitionSubResources(barriers, { cubePostDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		Texture::SubresouresTransitionInfo equirectangularPostDispatch;
		equirectangularPostDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		equirectangularPostDispatch.dstAccess = equirectangularTRI.dstAccess;
		equirectangularPostDispatch.oldLayout = equirectangularPreDispatch.newLayout;
		equirectangularPostDispatch.newLayout = equirectangularTRI.newLayout;
		equirectangularPostDispatch.subresoureRange = {};
		equirectangularPostDispatch.allSubresources = true;

		if (equirectangularPostDispatch.oldLayout != equirectangularPostDispatch.newLayout)
		{
			barriers.clear();
			equirectangularTRI.texture->TransitionSubResources(barriers, { equirectangularPostDispatch });
			m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		m_ComputeCmdBuffer->End(0);
	}

	m_ComputeCmdBuffer->Submit({ 0 }, {}, {}, {}, m_Fence);
	m_Fence->Wait();
}

void ImageProcessing::DiffuseIrradiance(const TextureResourceInfo& diffuseIrradianceTRI, const TextureResourceInfo& cubemapTRI)
{
	if (!s_PipelineDiffuseIrradiance)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/DiffuseIrradiance.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineDiffuseIrradiance = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	CommandPool::CreateInfo m_ComputeCmdPoolCI;
	m_ComputeCmdPoolCI.debugName = "GEAR_CORE_CommandPool_DiffuseIrradiance_Compute";
	m_ComputeCmdPoolCI.pContext = AllocatorManager::GetCreateInfo().pContext;
	m_ComputeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_ComputeCmdPoolCI.queueType = CommandPool::QueueType::COMPUTE;
	miru::Ref<CommandPool> m_ComputeCmdPool = CommandPool::Create(&m_ComputeCmdPoolCI);

	CommandBuffer::CreateInfo m_ComputeCmdBufferCI;
	m_ComputeCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_DiffuseIrradiance_Compute";
	m_ComputeCmdBufferCI.pCommandPool = m_ComputeCmdPool;
	m_ComputeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_ComputeCmdBufferCI.commandBufferCount = 1;
	m_ComputeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	miru::Ref<CommandBuffer> m_ComputeCmdBuffer = CommandBuffer::Create(&m_ComputeCmdBufferCI);

	miru::Ref<ImageView> m_EnviromentImageView;
	miru::Ref<ImageView> m_DiffuseIrradianceImageView;

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cubemap";
	m_ImageViewCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_ImageViewCI.pImage = cubemapTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_EnviromentImageView = ImageView::Create(&m_ImageViewCI);

	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_DiffuseIrradiance";
	m_ImageViewCI.pImage = diffuseIrradianceTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_DiffuseIrradianceImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_DiffuseIrradiance";
	m_DescPoolCI.device = m_ComputeCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1 }, {DescriptorType::STORAGE_IMAGE, 1 } };
	m_DescPoolCI.maxSets = 1;
	miru::Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	miru::Ref<DescriptorSet> m_DescSet;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_DiffuseIrradiance";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineEquirectangularToCube->GetDescriptorSetLayouts();
	m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	m_DescSet->AddImage(0, 0, { { cubemapTRI.texture->GetTextureSampler(), m_EnviromentImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	m_DescSet->AddImage(0, 1, { { nullptr, m_DiffuseIrradianceImageView, Image::Layout::GENERAL } });
	m_DescSet->Update();

	Fence::CreateInfo m_FenceCI;
	m_FenceCI.debugName = "GEAR_CORE_Fence_DiffuseIrradiance";
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
		toGeneral.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		toGeneral.subresoureRange = {};
		toGeneral.allSubresources = true;

		barriers.clear();
		cubemapTRI.texture->TransitionSubResources(barriers, { toGeneral });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindPipeline(0, s_PipelineEquirectangularToCube->GetPipeline());
		Texture::SubresouresTransitionInfo preDispatch;
		preDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		preDispatch.oldLayout = GraphicsAPI::IsVulkan() ? Image::Layout::UNKNOWN : Image::Layout::GENERAL;
		preDispatch.newLayout = Image::Layout::GENERAL;
		preDispatch.subresoureRange = {};
		preDispatch.allSubresources = true;

		barriers.clear();
		diffuseIrradianceTRI.texture->TransitionSubResources(barriers, { preDispatch });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineEquirectangularToCube->GetPipeline());
		uint32_t width = std::max(diffuseIrradianceTRI.texture->GetWidth() / 32, uint32_t(1));
		uint32_t height = std::max(diffuseIrradianceTRI.texture->GetHeight() / 32, uint32_t(1));
		uint32_t depth = 6;
		m_ComputeCmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo postDispatch;
		postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		postDispatch.oldLayout = Image::Layout::GENERAL;
		postDispatch.newLayout = GraphicsAPI::IsVulkan() ? Image::Layout::SHADER_READ_ONLY_OPTIMAL : Image::Layout::GENERAL;
		postDispatch.subresoureRange = {};
		postDispatch.allSubresources = true;

		barriers.clear();
		diffuseIrradianceTRI.texture->TransitionSubResources(barriers, { postDispatch });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);


		Texture::SubresouresTransitionInfo toTransferDst;
		toTransferDst.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		toTransferDst.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		toTransferDst.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		toTransferDst.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		toTransferDst.subresoureRange = {};
		toTransferDst.allSubresources = true;

		barriers.clear();
		cubemapTRI.texture->TransitionSubResources(barriers, { toTransferDst });
		m_ComputeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);

		m_ComputeCmdBuffer->End(0);
	}

	m_ComputeCmdBuffer->Submit({ 0 }, {}, {}, {}, m_Fence);
	m_Fence->Wait();
}

void ImageProcessing::RecompileRenderPipelineShaders()
{
	AllocatorManager::GetCreateInfo().pContext->DeviceWaitIdle();
	if(s_PipelineMipMap)
		s_PipelineMipMap->RecompileShaders();
	if (s_PipelineMipMapArray)
		s_PipelineMipMapArray->RecompileShaders();
	if (s_PipelineEquirectangularToCube)
		s_PipelineEquirectangularToCube->RecompileShaders();
	if (s_PipelineDiffuseIrradiance)
		s_PipelineDiffuseIrradiance->RecompileShaders();
}
