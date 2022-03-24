#pragma once
#include "ComponentUI.h"

namespace gear
{
namespace ui
{
namespace componentui
{
	void DrawLightComponentUI(gear::scene::Entity entity);
	void AddLightComponent(gear::scene::Entity entity, void* device);
	
	void DrawProbeComponentUI(Ref<objects::Probe> probe);
}
}
}
