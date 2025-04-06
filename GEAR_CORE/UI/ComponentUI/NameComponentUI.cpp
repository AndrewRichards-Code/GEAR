#include "gear_core_common.h"
#include "UI/ComponentUI/NameComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;

using namespace ui;
using namespace componentui;

void gear::ui::componentui::DrawNameComponentUI(Entity entity)
{
	DrawInputText("Name", entity.GetComponent<NameComponent>().name);
	DrawStaticText("UUID", std::to_string(entity.GetUUID()));
}