#pragma once

namespace gear
{
	namespace graphics
	{
		class RenderPipeline;
		class Texture;
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
			void DrawMaterialComponentUI(Ref<objects::Material>& material, UIContext* uiContext, bool fileFunctions = true);
			void DrawRenderPipelineComponentUI(Ref<graphics::RenderPipeline>& renderPipeline);
		}
	}
}
