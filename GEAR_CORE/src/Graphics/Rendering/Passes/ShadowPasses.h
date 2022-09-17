#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects { class Light; }

	namespace graphics
	{
		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class ShadowPasses
				{
				public:
					static void Main(Renderer& renderer, Ref<objects::Light> light);
					static void DebugShowDepth(Renderer& renderer, Ref<objects::Light> light);
				};
			}
		}
	}
}