#pragma once
#include "ComponentUI.h"

namespace gearbox
{
	namespace componentui
	{
		void DrawSkyboxComponentUI(gear::scene::Entity entity);
		void AddSkyboxComponent(gear::scene::Entity entity, void* device);
	}
}