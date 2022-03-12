#pragma once
#include "ComponentUI.h"
#include "Objects/Material.h"

namespace gear
{
namespace ui
{
namespace componentui
{
	void DrawMaterialUI(Ref<gear::objects::Material>& material, UIContext* uiContext, bool fileFunctions = true);
	void DrawRenderPipelineUI(gear::graphics::RenderPipeline* renderPipeline);
}
}
}
