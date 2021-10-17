#pragma once
#include "ComponentUI.h"

namespace gearbox
{
	namespace componentui
	{
		void DrawModelComponentUI(gear::scene::Entity entity, Ref<UIContext>& uiContext);
		void AddModelComponent(gear::scene::Entity entity, void* device);

		void DrawMeshUI(Ref<gear::objects::Mesh>& mesh, Ref<UIContext>& uiContext);
		void DrawMaterialUI(Ref<gear::objects::Material>& material, Ref<UIContext>& uiContext, bool fileFunctions = true);
	}
}
