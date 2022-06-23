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
		namespace componentui
		{
			void DrawModelComponentUI(scene::Entity entity);
			void AddModelComponent(scene::Entity entity, void* device);

			void DrawMeshUI(Ref<objects::Mesh>& mesh);
		}
	}
}
