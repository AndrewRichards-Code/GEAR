#pragma once
#include "ComponentUI.h"

namespace gear
{
namespace ui
{
namespace componentui
{
	void DrawCameraComponentUI(gear::scene::Entity entity, float screenRatio);
	void AddCameraComponent(gear::scene::Entity entity, float screenRatio, void* device);
}
}
}