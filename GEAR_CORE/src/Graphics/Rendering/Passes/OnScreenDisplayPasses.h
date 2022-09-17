#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class OnScreenDisplayPasses
				{
				public:
					static void Text(Renderer& renderer);
					static void CoordinateAxes(Renderer& renderer);
				};
			}
		}
	}
}