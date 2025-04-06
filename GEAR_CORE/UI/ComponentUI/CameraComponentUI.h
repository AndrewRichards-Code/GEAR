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
			void DrawCameraComponentUI(scene::Entity entity);
			void AddCameraComponent(scene::Entity entity, void* device);
		}
	}
}