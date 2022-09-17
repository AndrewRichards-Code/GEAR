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
				class MipmapPasses
				{
				public:
					static void GenerateMipmaps(Renderer& renderer, Ref<Texture> texture);
				};
			}
		}
	}
}