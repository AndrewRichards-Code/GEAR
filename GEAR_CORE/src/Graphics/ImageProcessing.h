#pragma once
#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class Texture;
	class RenderPipeline;

	class ImageProcessing
	{
	public:
		struct TextureResourceInfo
		{
			gear::Ref<Texture>&						texture;
			miru::crossplatform::Barrier::AccessBit srcAccess;
			miru::crossplatform::Image::Layout		oldLayout;
			miru::crossplatform::PipelineStageBit	srcStage;
		};

	private:
		static gear::Ref<RenderPipeline> s_PipelineMipMap;
		static gear::Ref<RenderPipeline> s_PipelineMipMapArray;
		static gear::Ref<RenderPipeline> s_PipelineEquirectangularToCube;
		static gear::Ref<RenderPipeline> s_PipelineDiffuseIrradiance;
		static gear::Ref<RenderPipeline> s_PipelineSpecularIrradiance;
		static gear::Ref<RenderPipeline> s_PipelineSpecularBRDF_LUT;

	public:
		ImageProcessing();
		~ImageProcessing();

		//Final state of the resource will be miru::crossplatform::Image::Layout::GENERAL
		static void GenerateMipMaps(const TextureResourceInfo& TRI);

		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void EquirectangularToCube(const TextureResourceInfo& environmentCubemapTRI, const TextureResourceInfo& equirectangularTRI);

		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void DiffuseIrradiance(const TextureResourceInfo& diffuseIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI);

		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void SpecularIrradiance(const TextureResourceInfo& specularIrradianceTRI, const TextureResourceInfo& environmentCubemapTRI);

		//Final state of the resources will be miru::crossplatform::Image::Layout::GENERAL
		static void SpecularBRDF_LUT(const TextureResourceInfo& TRI);

		static void RecompileRenderPipelineShaders();
	};
}
}
