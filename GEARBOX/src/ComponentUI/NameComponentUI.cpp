#include "gearbox_common.h"
#include "NameComponentUI.h"
#include "ComponentUI.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

void gearbox::componentui::DrawNameComponentUI(gear::scene::Entity entity)
{
	if (DrawTreeNode<NameComponent>("Name", entity))
	{
		DrawInputText("Name", entity.GetComponent<NameComponent>().name);
		DrawStaticText("UUID", std::to_string(entity.GetUUID()));
		EndDrawTreeNode();
	}
}