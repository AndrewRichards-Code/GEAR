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
			miru::crossplatform::Barrier::AccessBit dstAccess;
			miru::crossplatform::Image::Layout		oldLayout;
			miru::crossplatform::Image::Layout		newLayout;
		};

	private:
		static gear::Ref<RenderPipeline> s_PipelineMipMap;
		static gear::Ref<RenderPipeline> s_PipelineMipMapArray;
		static gear::Ref<RenderPipeline> s_PipelineEquirectangularToCube;
		static gear::Ref<RenderPipeline> s_PipelineDiffuseIrradiance;

	public:
		ImageProcessing();
		~ImageProcessing();

		static void GenerateMipMaps(const TextureResourceInfo& TRI);
		static void EquirectangularToCube(const TextureResourceInfo& cubeTRI, const TextureResourceInfo& equirectangularTRI);
		static void DiffuseIrradiance(const TextureResourceInfo& diffuseIrradianceTRI, const TextureResourceInfo& cubemapTRI);
		//static void SpecularIrradiance(const TextureResourceInfo& specularIrradianceTRI, const TextureResourceInfo& cubemapTRI);
		//static void SpecularIrradiance(const TextureResourceInfo& TRI);

		static void RecompileRenderPipelineShaders();
	};
}
}
