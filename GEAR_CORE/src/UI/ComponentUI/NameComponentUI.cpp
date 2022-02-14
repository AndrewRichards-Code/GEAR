#include "gear_core_common.h"
#include "NameComponentUI.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace ui;
using namespace componentui;

void gear::ui::componentui::DrawNameComponentUI(gear::scene::Entity entity)
{
	DrawInputText("Name", entity.GetComponent<NameComponent>().name);
	DrawStaticText("UUID", std::to_string(entity.GetUUID()));
}