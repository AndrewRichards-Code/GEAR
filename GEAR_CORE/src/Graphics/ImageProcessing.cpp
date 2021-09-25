#include "gear_core_common.h"
#include "ImageProcessing.h"

#include "AllocatorManager.h"
#include "RenderPipeline.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "UniformBufferStructures.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Ref<RenderPipeline> ImageProcessing::s_PipelineMipMap;
Ref<RenderPipeline> ImageProcessing::s_PipelineMipMapArray;
Ref<RenderPipeline> ImageProcessing::s_PipelineEquirectangularToCube;
Ref<RenderPipeline> ImageProcessing::s_PipelineDiffuseIrradiance;
Ref<RenderPipeline> ImageProcessing::s_PipelineSpecularIrradiance;
Ref<RenderPipeline> ImageProcessing::s_PipelineSpecularBRDF_LUT;

std::vector<Ref<ImageView>>		ImageProcessing::s_ImageViews;
std::vector<Ref<DescriptorSet>>	ImageProcessing::s_DescSets;

typedef UniformBufferStructures::SpecularIrradianceInfo SpecularIrradianceInfoUB;
static std::vector<Ref<Uniformbuffer<SpecularIrradianceInfoUB>>> m_SpecularIrradianceInfoUBs;

ImageProcessing::ImageProcessing()
{
}

ImageProcessing::~ImageProcessing()
{
}

void ImageProcessing::GenerateMipMaps(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& TRI)
{
	if (!TRI.texture->m_GenerateMipMaps || TRI.texture->m_Generated)
		return;

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

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();
	const uint32_t& levels = TRI.texture->GetCreateInfo().mipLevels;
	const uint32_t& layers = TRI.texture->GetCreateInfo().arrayLayers;

	Ref<RenderPipeline> pipeline = s_PipelineMipMap;
	if (layers > 1)
		pipeline = s_PipelineMipMapArray;

	std::vector<Ref<ImageView>> m_ImageViews;
	m_ImageViews.reserve(levels);

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_MipMap";
	m_ImageViewCI.device = device;
	m_ImageViewCI.pImage = TRI.texture->GetTexture();
	m_ImageViewCI.viewType = pipeline == s_PipelineMipMapArray ? Image::Type::TYPE_2D_ARRAY : Image::Type::TYPE_2D;
	for (uint32_t i = 0; i < levels; i++)
	{
		m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
		m_ImageViews.emplace_back(ImageView::Create(&m_ImageViewCI));
	}

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_MipMap";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::SAMPLED_IMAGE, (levels - 1)}, {DescriptorType::STORAGE_IMAGE, (levels - 1)} };
	m_DescPoolCI.maxSets = levels - 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	std::vector<Ref<DescriptorSet>> m_DescSets;
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

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(m_ImageViews, m_DescSets);

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo texturePreDispatch;
		texturePreDispatch.srcAccess = TRI.srcAccess;
		texturePreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		texturePreDispatch.oldLayout = TRI.oldLayout;
		texturePreDispatch.newLayout = m_ImageLayout;
		texturePreDispatch.subresourceRange = {};
		texturePreDispatch.allSubresources = true;
		
		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { texturePreDispatch });
		cmdBuffer->PipelineBarrier(0, TRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindPipeline(0, pipeline->GetPipeline());
		for (uint32_t i = 1; i < levels; i++)
		{
			Texture::SubresouresTransitionInfo preDispatch;
			preDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			preDispatch.oldLayout = m_ImageLayout;
			preDispatch.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : m_ImageLayout;
			preDispatch.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			preDispatch.allSubresources = false;

			barriers.clear();
			TRI.texture->TransitionSubResources(barriers, { preDispatch });
			cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

			cmdBuffer->BindDescriptorSets(0, { m_DescSets[i - 1] }, pipeline->GetPipeline());
			uint32_t width = std::max((TRI.texture->GetWidth() >> i) / 8, uint32_t(1));
			uint32_t height = std::max((TRI.texture->GetHeight() >> i) / 8, uint32_t(1));
			uint32_t depth = layers;
			cmdBuffer->Dispatch(0, width, height, depth);

			Texture::SubresouresTransitionInfo postDispatch;
			postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
			postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			postDispatch.oldLayout = preDispatch.newLayout;
			postDispatch.newLayout = m_ImageLayout;
			postDispatch.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers };
			postDispatch.allSubresources = false;

			barriers.clear();
			TRI.texture->TransitionSubResources(barriers, { postDispatch });
			cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}
	
		Texture::SubresouresTransitionInfo texturePostDispatch;
		texturePostDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		texturePostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		texturePostDispatch.oldLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : m_ImageLayout; //D3D12: D3D12_RESOURCE_STATE_COMMON is promoted to D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE after Dispatch() for SRV accesses.
		texturePostDispatch.newLayout = m_ImageLayout;
		texturePostDispatch.subresourceRange = {};
		texturePostDispatch.allSubresources = true;

		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { texturePostDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
	}
	
	TRI.texture->m_Generated = true;
}

void ImageProcessing::EquirectangularToCube(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& environmentCubemapTRI, const TextureResourceInfo& equirectangularTRI)
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

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();

	Ref<ImageView> m_EquirectangularImageView;
	Ref<ImageView> m_CubeImageView;

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Equirectangular";
	m_ImageViewCI.device = device;
	m_ImageViewCI.pImage = equirectangularTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_EquirectangularImageView= ImageView::Create(&m_ImageViewCI);

	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cube";
	m_ImageViewCI.pImage = environmentCubemapTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_CubeImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_EquirectangularToCube";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1 }, {DescriptorType::STORAGE_IMAGE, 1 } };
	m_DescPoolCI.maxSets = 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	Ref<DescriptorSet> m_DescSet;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_EquirectangularToCube";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineEquirectangularToCube->GetDescriptorSetLayouts();
	m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	Image::Layout m_EquirectangularImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	Image::Layout m_CubeImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	m_DescSet->AddImage(0, 0, { { equirectangularTRI.texture->GetTextureSampler(), m_EquirectangularImageView, m_EquirectangularImageLayout } });
	m_DescSet->AddImage(0, 1, { { nullptr, m_CubeImageView, m_CubeImageLayout } });
	m_DescSet->Update();

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(std::vector({ m_EquirectangularImageView, m_CubeImageView }), std::vector({ m_DescSet }));

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo equirectangularPreDispatch;
		equirectangularPreDispatch.srcAccess = equirectangularTRI.srcAccess;
		equirectangularPreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		equirectangularPreDispatch.oldLayout = equirectangularTRI.oldLayout;
		equirectangularPreDispatch.newLayout = m_EquirectangularImageLayout;
		equirectangularPreDispatch.subresourceRange = {};
		equirectangularPreDispatch.allSubresources = true;

		if (equirectangularPreDispatch.oldLayout != equirectangularPreDispatch.newLayout)
		{
			barriers.clear();
			equirectangularTRI.texture->TransitionSubResources(barriers, { equirectangularPreDispatch });
			cmdBuffer->PipelineBarrier(0, equirectangularTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		cmdBuffer->BindPipeline(0, s_PipelineEquirectangularToCube->GetPipeline());
		Texture::SubresouresTransitionInfo cubePreDispatch;
		cubePreDispatch.srcAccess = environmentCubemapTRI.srcAccess;
		cubePreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		cubePreDispatch.oldLayout = environmentCubemapTRI.oldLayout;
		cubePreDispatch.newLayout = m_CubeImageLayout;
		cubePreDispatch.subresourceRange = {};
		cubePreDispatch.allSubresources = true;

		if (cubePreDispatch.oldLayout != cubePreDispatch.newLayout)
		{
			barriers.clear();
			environmentCubemapTRI.texture->TransitionSubResources(barriers, { cubePreDispatch });
			cmdBuffer->PipelineBarrier(0, environmentCubemapTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		cmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineEquirectangularToCube->GetPipeline());
		uint32_t width = std::max(environmentCubemapTRI.texture->GetWidth() / 32, uint32_t(1));
		uint32_t height = std::max(environmentCubemapTRI.texture->GetHeight() / 32, uint32_t(1));
		uint32_t depth = 6;
		cmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo cubePostDispatch;
		cubePostDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		cubePostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		cubePostDispatch.oldLayout = m_CubeImageLayout; //D3D12: D3D12_RESOURCE_STATE_COMMON is promoted to D3D12_RESOURCE_STATE_UNORDERED_ACCESS after Dispatch() for UAV accesses.
		cubePostDispatch.newLayout = Image::Layout::GENERAL;
		cubePostDispatch.subresourceRange = {};
		cubePostDispatch.allSubresources = true;
		
		//if (cubePostDispatch.oldLayout != cubePostDispatch.newLayout)
		{
			barriers.clear();
			environmentCubemapTRI.texture->TransitionSubResources(barriers, { cubePostDispatch });
			cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}

		Texture::SubresouresTransitionInfo equirectangularPostDispatch;
		equirectangularPostDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		equirectangularPostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		equirectangularPostDispatch.oldLayout = equirectangularPreDispatch.newLayout;
		equirectangularPostDispatch.newLayout = Image::Layout::GENERAL;
		equirectangularPostDispatch.subresourceRange = {};
		equirectangularPostDispatch.allSubresources = true;

		//if (equirectangularPostDispatch.oldLayout != equirectangularPostDispatch.newLayout)
		{
			barriers.clear();
			equirectangularTRI.texture->TransitionSubResources(barriers, { equirectangularPostDispatch });
			cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		}
	}
}

void ImageProcessing::DiffuseIrradiance(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& diffuseIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI)
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

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();

	Ref<ImageView> m_EnvironmentImageView;
	Ref<ImageView> m_DiffuseIrradianceImageView;

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cubemap";
	m_ImageViewCI.device = device;
	m_ImageViewCI.pImage = environmentCubemapTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_EnvironmentImageView = ImageView::Create(&m_ImageViewCI);

	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_DiffuseIrradiance";
	m_ImageViewCI.pImage = diffuseIrradianceTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_DiffuseIrradianceImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_DiffuseIrradiance";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1 }, {DescriptorType::STORAGE_IMAGE, 1 } };
	m_DescPoolCI.maxSets = 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	Ref<DescriptorSet> m_DescSet;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_DiffuseIrradiance";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineDiffuseIrradiance->GetDescriptorSetLayouts();
	m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	Image::Layout m_EnvironmentImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	Image::Layout m_DiffuseIrradianceImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	m_DescSet->AddImage(0, 0, { { environmentCubemapTRI.texture->GetTextureSampler(), m_EnvironmentImageView, m_EnvironmentImageLayout } });
	m_DescSet->AddImage(0, 1, { { nullptr, m_DiffuseIrradianceImageView, m_DiffuseIrradianceImageLayout } });
	m_DescSet->Update();

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(std::vector({ m_EnvironmentImageView, m_DiffuseIrradianceImageView }), std::vector({ m_DescSet }));

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo environmentPreDispatch;
		environmentPreDispatch.srcAccess = environmentCubemapTRI.srcAccess;
		environmentPreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		environmentPreDispatch.oldLayout = environmentCubemapTRI.oldLayout;
		environmentPreDispatch.newLayout = m_EnvironmentImageLayout;
		environmentPreDispatch.subresourceRange = {};
		environmentPreDispatch.allSubresources = true;

		barriers.clear();
		environmentCubemapTRI.texture->TransitionSubResources(barriers, { environmentPreDispatch });
		cmdBuffer->PipelineBarrier(0, environmentCubemapTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindPipeline(0, s_PipelineDiffuseIrradiance->GetPipeline());
		Texture::SubresouresTransitionInfo diffuseCubePreDispatch;
		diffuseCubePreDispatch.srcAccess = diffuseIrradianceTRI.srcAccess;
		diffuseCubePreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		diffuseCubePreDispatch.oldLayout = diffuseIrradianceTRI.oldLayout;
		diffuseCubePreDispatch.newLayout = m_DiffuseIrradianceImageLayout;
		diffuseCubePreDispatch.subresourceRange = {};
		diffuseCubePreDispatch.allSubresources = true;

		barriers.clear();
		diffuseIrradianceTRI.texture->TransitionSubResources(barriers, { diffuseCubePreDispatch });
		cmdBuffer->PipelineBarrier(0, diffuseIrradianceTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineDiffuseIrradiance->GetPipeline());
		uint32_t width = std::max(diffuseIrradianceTRI.texture->GetWidth() / 32, uint32_t(1));
		uint32_t height = std::max(diffuseIrradianceTRI.texture->GetHeight() / 32, uint32_t(1));
		uint32_t depth = 6;
		cmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo diffuseCubePostDispatch;
		diffuseCubePostDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		diffuseCubePostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		diffuseCubePostDispatch.oldLayout = m_DiffuseIrradianceImageLayout;
		diffuseCubePostDispatch.newLayout = Image::Layout::GENERAL;
		diffuseCubePostDispatch.subresourceRange = {};
		diffuseCubePostDispatch.allSubresources = true;

		barriers.clear();
		diffuseIrradianceTRI.texture->TransitionSubResources(barriers, { diffuseCubePostDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		Texture::SubresouresTransitionInfo environmentPostDispatch;
		environmentPostDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		environmentPostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		environmentPostDispatch.oldLayout = environmentPreDispatch.newLayout;
		environmentPostDispatch.newLayout = Image::Layout::GENERAL;
		environmentPostDispatch.subresourceRange = {};
		environmentPostDispatch.allSubresources = true;

		barriers.clear();
		environmentCubemapTRI.texture->TransitionSubResources(barriers, { environmentPostDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
	}
}

void ImageProcessing::SpecularIrradiance(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& specularIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI)
{
	if (!s_PipelineSpecularIrradiance)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/SpecularIrradiance.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineSpecularIrradiance = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();

	const uint32_t& levels = specularIrradianceTRI.texture->GetCreateInfo().mipLevels;

	Ref<ImageView> m_EnvironmentImageView;
	std::vector<Ref<ImageView>> m_SpecularIrradianceImageViews;
	m_SpecularIrradianceImageViews.reserve(levels);

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_Cubemap";
	m_ImageViewCI.device = device;
	m_ImageViewCI.pImage = environmentCubemapTRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_CUBE;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	m_EnvironmentImageView = ImageView::Create(&m_ImageViewCI);

	for (uint32_t i = 0; i < levels; i++)
	{
		m_ImageViewCI.debugName = "GEAR_CORE_ImageView_SpecularIrradiance";
		m_ImageViewCI.pImage = specularIrradianceTRI.texture->GetTexture();
		m_ImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
		m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 6 };
		m_SpecularIrradianceImageViews.push_back(ImageView::Create(&m_ImageViewCI));
	}

	m_SpecularIrradianceInfoUBs.clear();
	m_SpecularIrradianceInfoUBs.resize(levels);

	float zero[sizeof(SpecularIrradianceInfoUB)] = { 0 };
	Uniformbuffer<SpecularIrradianceInfoUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Buffer_SpecularIrradianceInfoUB";
	ubCI.device = device;
	ubCI.data = zero;
	for (uint32_t i = 0; i < levels; i++)
	{
		m_SpecularIrradianceInfoUBs[i] = CreateRef<Uniformbuffer<SpecularIrradianceInfoUB>>(&ubCI);
		m_SpecularIrradianceInfoUBs[i]->roughness = float(i) / float(levels);
		m_SpecularIrradianceInfoUBs[i]->SubmitData();
	}

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_SpecularIrradiance";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1 }, {DescriptorType::STORAGE_IMAGE, 1 },  { DescriptorType::UNIFORM_BUFFER, 1 } };
	m_DescPoolCI.maxSets = levels;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	std::vector<Ref<DescriptorSet>> m_DescSets;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_SpecularIrradiance";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineSpecularIrradiance->GetDescriptorSetLayouts();
	Image::Layout m_EnvironmentImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL : Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	Image::Layout m_SpecularIrradianceImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	for (uint32_t i = 0; i < m_DescPoolCI.maxSets; i++)
	{
		m_DescSets.emplace_back(DescriptorSet::Create(&m_DescSetCI));
		m_DescSets[i]->AddImage(0, 0, { { environmentCubemapTRI.texture->GetTextureSampler(), m_EnvironmentImageView, m_EnvironmentImageLayout } });
		m_DescSets[i]->AddImage(0, 1, { { nullptr, m_SpecularIrradianceImageViews[i], m_SpecularIrradianceImageLayout } });
		m_DescSets[i]->AddBuffer(0, 2, { { m_SpecularIrradianceInfoUBs[i]->GetBufferView() } });
		m_DescSets[i]->Update();
	}
	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(m_SpecularIrradianceImageViews, m_DescSets);
	s_ImageViews.push_back(m_EnvironmentImageView);

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo environmentPreDispatch;
		environmentPreDispatch.srcAccess = environmentCubemapTRI.srcAccess;
		environmentPreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		environmentPreDispatch.oldLayout = environmentCubemapTRI.oldLayout;
		environmentPreDispatch.newLayout = m_EnvironmentImageLayout;
		environmentPreDispatch.subresourceRange = {};
		environmentPreDispatch.allSubresources = true;

		barriers.clear();
		environmentCubemapTRI.texture->TransitionSubResources(barriers, { environmentPreDispatch });
		cmdBuffer->PipelineBarrier(0, environmentCubemapTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindPipeline(0, s_PipelineSpecularIrradiance->GetPipeline());
		Texture::SubresouresTransitionInfo specularCubePreDispatch;
		specularCubePreDispatch.srcAccess = specularIrradianceTRI.srcAccess;
		specularCubePreDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		specularCubePreDispatch.oldLayout = specularIrradianceTRI.oldLayout;
		specularCubePreDispatch.newLayout = m_SpecularIrradianceImageLayout;
		specularCubePreDispatch.subresourceRange = {};
		specularCubePreDispatch.allSubresources = true;

		barriers.clear();
		specularIrradianceTRI.texture->TransitionSubResources(barriers, { specularCubePreDispatch });
		cmdBuffer->PipelineBarrier(0, specularIrradianceTRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		
		for (uint32_t i = 0; i < levels; i++)
		{
			m_SpecularIrradianceInfoUBs[i]->Upload(cmdBuffer, 0, true);
			if (GraphicsAPI::IsD3D12())
			{
				Barrier::CreateInfo bCI;
				bCI.type = Barrier::Type::BUFFER;
				bCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
				bCI.dstAccess = Barrier::AccessBit::UNIFORM_READ_BIT;
				bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
				bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
				bCI.pBuffer = m_SpecularIrradianceInfoUBs[i]->GetBuffer();
				bCI.offset = 0;
				bCI.size = m_SpecularIrradianceInfoUBs[i]->GetSize();
				Ref<Barrier> b = Barrier::Create(&bCI);
				cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, { b });
			};

			cmdBuffer->BindDescriptorSets(0, { m_DescSets[i] }, s_PipelineSpecularIrradiance->GetPipeline());
			uint32_t width = std::max((specularIrradianceTRI.texture->GetWidth() >> i) / 32, uint32_t(1));
			uint32_t height = std::max((specularIrradianceTRI.texture->GetHeight() >> i) / 32, uint32_t(1));
			uint32_t depth = 6;
			cmdBuffer->Dispatch(0, width, height, depth);
		}

		Texture::SubresouresTransitionInfo specularCubePostDispatch;
		specularCubePostDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		specularCubePostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		specularCubePostDispatch.oldLayout = m_SpecularIrradianceImageLayout;
		specularCubePostDispatch.newLayout = Image::Layout::GENERAL;
		specularCubePostDispatch.subresourceRange = {};
		specularCubePostDispatch.allSubresources = true;

		barriers.clear();
		specularIrradianceTRI.texture->TransitionSubResources(barriers, { specularCubePostDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		Texture::SubresouresTransitionInfo environmentPostDispatch;
		environmentPostDispatch.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		environmentPostDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		environmentPostDispatch.oldLayout = environmentPreDispatch.newLayout;
		environmentPostDispatch.newLayout = Image::Layout::GENERAL;
		environmentPostDispatch.subresourceRange = {};
		environmentPostDispatch.allSubresources = true;

		barriers.clear();
		environmentCubemapTRI.texture->TransitionSubResources(barriers, { environmentPostDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
	}
}

void ImageProcessing::SpecularBRDF_LUT(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& TRI)
{
	if (!s_PipelineSpecularBRDF_LUT)
	{
		RenderPipeline::LoadInfo s_PipelineLI;
		s_PipelineLI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		s_PipelineLI.filepath = "res/pipelines/SpecularBRDF_LUT.grpf.json";
		s_PipelineLI.viewportWidth = 0.0f;
		s_PipelineLI.viewportHeight = 0.0f;
		s_PipelineLI.renderPass = nullptr;
		s_PipelineLI.subpassIndex = 0;
		s_PipelineSpecularBRDF_LUT = CreateRef<RenderPipeline>(&s_PipelineLI);
	}

	void* device = cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().pContext->GetDevice();

	Ref<ImageView> m_SpecularBRDF_LUTImageView;

	ImageView::CreateInfo m_ImageViewCI;
	m_ImageViewCI.debugName = "GEAR_CORE_ImageView_SpecularBRDF_LUT";
	m_ImageViewCI.device = device;
	m_ImageViewCI.pImage = TRI.texture->GetTexture();
	m_ImageViewCI.viewType = Image::Type::TYPE_2D;
	m_ImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	m_SpecularBRDF_LUTImageView = ImageView::Create(&m_ImageViewCI);

	DescriptorPool::CreateInfo m_DescPoolCI;
	m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_SpecularBRDF_LUT";
	m_DescPoolCI.device = device;
	m_DescPoolCI.poolSizes = { { DescriptorType::STORAGE_IMAGE, 1 } };
	m_DescPoolCI.maxSets = 1;
	Ref<DescriptorPool> m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	Ref<DescriptorSet> m_DescSet;
	DescriptorSet::CreateInfo m_DescSetCI;
	m_DescSetCI.debugName = "GEAR_CORE_DescriptorSet_SpecularBRDF_LUT";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = s_PipelineSpecularBRDF_LUT->GetDescriptorSetLayouts();
	m_DescSet = DescriptorSet::Create(&m_DescSetCI);
	Image::Layout m_SpecularBRDF_LUTImageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
	m_DescSet->AddImage(0, 0, { { nullptr, m_SpecularBRDF_LUTImageView, m_SpecularBRDF_LUTImageLayout } });
	m_DescSet->Update();

	//Save the Image Views and Descriptor Set as the command buffer is executed out of scope.
	SaveImageViewsAndDescriptorSets(std::vector({ m_SpecularBRDF_LUTImageView }), std::vector({ m_DescSet }));

	//Record Compute CommandBuffer
	{
		std::vector<Ref<Barrier>> barriers;

		Texture::SubresouresTransitionInfo preDispatch;
		preDispatch.srcAccess = TRI.srcAccess;
		preDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		preDispatch.oldLayout = TRI.oldLayout;
		preDispatch.newLayout = m_SpecularBRDF_LUTImageLayout;
		preDispatch.subresourceRange = {};
		preDispatch.allSubresources = true;

		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { preDispatch });
		cmdBuffer->PipelineBarrier(0, TRI.srcStage, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);

		cmdBuffer->BindPipeline(0, s_PipelineSpecularBRDF_LUT->GetPipeline());
		cmdBuffer->BindDescriptorSets(0, { m_DescSet }, s_PipelineSpecularBRDF_LUT->GetPipeline());

		uint32_t width = std::max(TRI.texture->GetWidth() / 32, uint32_t(1));
		uint32_t height = std::max(TRI.texture->GetHeight() / 32, uint32_t(1));
		uint32_t depth = 1;
		cmdBuffer->Dispatch(0, width, height, depth);

		Texture::SubresouresTransitionInfo postDispatch;
		postDispatch.srcAccess = Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT;
		postDispatch.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		postDispatch.oldLayout = m_SpecularBRDF_LUTImageLayout;
		postDispatch.newLayout = Image::Layout::GENERAL;
		postDispatch.subresourceRange = {};
		postDispatch.allSubresources = true;

		barriers.clear();
		TRI.texture->TransitionSubResources(barriers, { postDispatch });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
	}
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
	if (s_PipelineSpecularIrradiance)
		s_PipelineSpecularIrradiance->RecompileShaders();
}
