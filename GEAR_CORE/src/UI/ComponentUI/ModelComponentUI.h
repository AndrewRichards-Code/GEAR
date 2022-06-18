#pragma once

namespace gear
{
	namespace objects
	{
		class Mesh;
	}
	namespace scene
	{
		class Entity;
	}
	namespace ui
	{
		class UIContext;

		namespace componentui
		{
			void DrawModelComponentUI(scene::Entity entity, UIContext* uiContext);
			void AddModelComponent(scene::Entity entity, void* device);

			void DrawMeshUI(Ref<objects::Mesh>& mesh, UIContext* uiContext);
		}
	}
}
