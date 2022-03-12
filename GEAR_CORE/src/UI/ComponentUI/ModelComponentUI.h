#pragma once
#include "ComponentUI.h"

namespace gear
{
namespace ui
{
namespace componentui
{
	void DrawModelComponentUI(gear::scene::Entity entity, UIContext* uiContext);
	void AddModelComponent(gear::scene::Entity entity, void* device);

	void DrawMeshUI(Ref<gear::objects::Mesh>& mesh, UIContext* uiContext);
}
}
}
