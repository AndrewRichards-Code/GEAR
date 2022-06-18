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
			void DrawCameraComponentUI(scene::Entity entity, float screenRatio);
			void AddCameraComponent(scene::Entity entity, float screenRatio, void* device);
		}
	}
}