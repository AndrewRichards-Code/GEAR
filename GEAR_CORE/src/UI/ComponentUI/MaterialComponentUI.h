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
			void DrawMaterialUI(Ref<objects::Material>& material, UIContext* uiContext, bool fileFunctions = true);
			void DrawTextureUI(Ref<graphics::Texture>& texture);
			void DrawRenderPipelineUI(Ref<graphics::RenderPipeline>& renderPipeline);
		}
	}
}
