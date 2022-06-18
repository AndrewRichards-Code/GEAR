#pragma once

namespace gear
{
	namespace graphics
	{
		class RenderPipeline;
	}
	namespace objects
	{
		class Material;
	}
	namespace ui
	{
		class UIContext;

		namespace componentui
		{
			void DrawMaterialUI(Ref<objects::Material>& material, UIContext* uiContext, bool fileFunctions = true);
			void DrawRenderPipelineUI(graphics::RenderPipeline* renderPipeline);
		}
	}
}
