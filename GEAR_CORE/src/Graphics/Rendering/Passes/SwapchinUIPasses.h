#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace ui { class UIContext; }

	namespace graphics
	{
		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class SwapchinUIPasses
				{
				public:
					static void CopyToSwapchain(Renderer& renderer);
					static void ExternalUI(Renderer& renderer, ui::UIContext* uiContext);
				};
			}
		}
	}
}