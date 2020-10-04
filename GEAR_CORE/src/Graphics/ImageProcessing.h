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
	private:
		static gear::Ref<RenderPipeline> s_PipelineMipMap;
		static gear::Ref<RenderPipeline> s_PipelineMipMapArray;
		static gear::Ref<RenderPipeline> s_PipelineEquirectangularToCube;

	public:
		ImageProcessing();
		~ImageProcessing();

		static void GenerateMipMaps(Texture* texture);
		static void EquirectangularToCube(gear::Ref<Texture>& cubemap, gear::Ref<Texture>& texture);
	};
}
}
