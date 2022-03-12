#pragma once
#include "gear_core_common.h"
#include "UniformBuffer.h"

namespace gear 
{
namespace graphics 
{
	class Texture;
	class RenderPipeline;

	class GEAR_API ImageProcessing
	{
	public:
		struct TextureResourceInfo
		{
			Ref<Texture>							texture;
			miru::crossplatform::Barrier::AccessBit srcAccess;
			miru::crossplatform::Image::Layout		oldLayout;
			miru::crossplatform::PipelineStageBit	srcStage;
		};

	private:
		static Ref<RenderPipeline> s_PipelineMipMap;
		static Ref<RenderPipeline> s_PipelineMipMapArray;
		static Ref<RenderPipeline> s_PipelineEquirectangularToCube;
		static Ref<RenderPipeline> s_PipelineDiffuseIrradiance;
		static Ref<RenderPipeline> s_PipelineSpecularIrradiance;
		static Ref<RenderPipeline> s_PipelineSpecularBRDF_LUT;

	public:
		static void InitialiseRenderPipelines();
		static void UninitialiseRenderPipelines();

		//Final state of the resource will be miru::crossplatform::Image::Layout::GENERAL
		static void GenerateMipMaps(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& TRI);
		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void EquirectangularToCube(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& environmentCubemapTRI, const TextureResourceInfo& equirectangularTRI);
		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void DiffuseIrradiance(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& diffuseIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI);
		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void SpecularIrradiance(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& specularIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI);
		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void SpecularBRDF_LUT(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& TRI);
		static void RecompileRenderPipelineShaders();

	public:
		typedef void(*PFN_ImageProcessing1)(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& tri1);
		typedef void(*PFN_ImageProcessing2)(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, const TextureResourceInfo& tri1, const TextureResourceInfo& tri2);

	public:
		static std::vector<Ref<miru::crossplatform::ImageView>> s_ImageViews;
		static std::vector<Ref<miru::crossplatform::DescriptorSet>> s_DescSets;

		typedef  gear::graphics::UniformBufferStructures::SpecularIrradianceInfo SpecularIrradianceInfoUB;
		static std::vector<Ref<Uniformbuffer<SpecularIrradianceInfoUB>>> s_SpecularIrradianceInfoUBs;

		static void SaveImageViewsAndDescriptorSets(const std::vector<Ref<miru::crossplatform::ImageView>>& imageViews, const std::vector<Ref<miru::crossplatform::DescriptorSet>>& descSets)
		{
			for (const auto& imageView : imageViews)
				s_ImageViews.push_back(imageView);
			for (const auto& descSet : descSets)
				s_DescSets.push_back(descSet);
		}
		static void ClearImageViewsAndDescriptorSets()
		{
			for (auto& imageView : s_ImageViews)
				imageView = nullptr;
			for (auto& descSet : s_DescSets)
				descSet = nullptr;
			
			s_ImageViews.clear();
			s_DescSets.clear();
		}
		static void ClearState()
		{
			ClearImageViewsAndDescriptorSets();

			for (auto& specularIrradianceInfoUB : s_SpecularIrradianceInfoUBs)
				specularIrradianceInfoUB = nullptr;
			
			s_SpecularIrradianceInfoUBs.clear();
		}
	};
}
}
