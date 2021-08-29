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
			Ref<Texture>&							texture;
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
