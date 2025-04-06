#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		class Texture;

		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class PostProcessingPasses
				{
				public:
					class Bloom
					{
					public:
						static void Prefilter(Renderer& renderer);
						static void Downsample(Renderer& renderer);
						static void Upsample(Renderer& renderer);
					};
					class HDRMapping
					{
					public:
						static void Main(Renderer& renderer);
					};
				};
			}
		}
	}
}