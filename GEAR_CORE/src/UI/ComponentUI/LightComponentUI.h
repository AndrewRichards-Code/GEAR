#pragma once
#include "ComponentUI.h"

namespace gear
{
	namespace objects
	{
		class Probe;
	}
	namespace scene
	{
		class Entity;
	}
	namespace ui
	{
		namespace componentui
		{
			void DrawLightComponentUI(scene::Entity entity);
			void AddLightComponent(scene::Entity entity, void* device);

			void DrawProbeComponentUI(Ref<objects::Probe> probe);
		}
	}
}
