#pragma once
#include "ComponentUI.h"

namespace gearbox
{
	namespace componentui
	{
		void DrawCameraComponentUI(gear::scene::Entity entity, float screenRatio);
		void AddCameraComponent(gear::scene::Entity entity, float screenRatio, void* device);
	}
}