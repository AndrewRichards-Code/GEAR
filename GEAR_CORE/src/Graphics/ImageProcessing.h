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
		static gear::Ref<RenderPipeline> s_PipelineMipMaps;

	public:
		ImageProcessing();
		~ImageProcessing();

		static void GenerateMipMaps(Texture* texture);

	};
}
}
