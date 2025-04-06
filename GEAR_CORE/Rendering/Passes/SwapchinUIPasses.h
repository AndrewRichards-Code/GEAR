#pragma once
#include "gear_core_common.h"
#include "Rendering/Renderer.h"

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
					static void ExternalUI(Renderer& renderer, ui::UIContext* uiContext, Renderer::PFN_SetPassParameters pfnSetPassParameters, Renderer::PFN_RenderDrawData pfnRenderDrawData);
				};
			}
		}
	}
}