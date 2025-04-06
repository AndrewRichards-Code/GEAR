#pragma once

namespace gear
{
	namespace scene
	{
		class Entity;
	}
	namespace ui
	{
		namespace componentui
		{
			void DrawSkyboxComponentUI(scene::Entity entity);
			void AddSkyboxComponent(scene::Entity entity, void* device);
		}
	}
}
