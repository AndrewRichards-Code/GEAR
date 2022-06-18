#include "gear_core_common.h"
#include "UI/ComponentUI/TransformComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gear;
using namespace ui;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawTransformComponentUI(Entity entity)
{
	objects::Transform& transform = entity.GetComponent<TransformComponent>().transform;

	//Translation
	DrawFloat3("Translation", transform.translation);
	
	//Rotation
	static bool radians = false;
	static bool quaternion = false;
	if (quaternion)
	{
		DrawQuaternion("Rotation", transform.orientation);
		transform.orientation.Normalise();
	}
	else
	{
		float3 ea = Quaternion::ToEulerAngles<float>(transform.orientation);
		if (!radians)
		{
			float3 _ea = float3(RadToDeg(ea.x), RadToDeg(ea.y), RadToDeg(ea.z));
			ea = _ea;
		}
		DrawFloat3("Rotation", ea);
		if (!radians)
		{
			float3 _ea = float3(DegToRad(ea.x), DegToRad(ea.y), DegToRad(ea.z));
			ea = _ea;
		}

		transform.orientation = Quaternion::FromEulerAngles(ea);
		transform.orientation.Normalise();
	}

	//Scale
	DrawFloat3("Scale", transform.scale, 1.0f);
	static bool scaleLock = false;
	if (scaleLock)
	{
		bool xy = transform.scale.x == transform.scale.y;
		bool yz = transform.scale.y == transform.scale.z;
		bool zx = transform.scale.z == transform.scale.x;

		if (xy) { transform.scale.x = transform.scale.y = transform.scale.z; }
		if (yz) { transform.scale.y = transform.scale.z = transform.scale.x; }
		if (zx) { transform.scale.z = transform.scale.x = transform.scale.y; }

	}

	if (DrawTreeNode("Options", false))
	{
		DrawCheckbox("Radians", radians);
		DrawCheckbox("Quaternion", quaternion);
		DrawCheckbox("Scale Lock", scaleLock);

		EndDrawTreeNode();
	}
}
