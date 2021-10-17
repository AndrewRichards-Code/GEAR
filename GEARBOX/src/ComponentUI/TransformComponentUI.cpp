#include "gearbox_common.h"
#include "TransformComponentUI.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

using namespace mars;

void gearbox::componentui::DrawTransformComponentUI(gear::scene::Entity entity)
{
	objects::Transform& transform = entity.GetComponent<TransformComponent>().transform;

	//Translation
	DrawVec3("Translation", transform.translation);
	
	//Rotation
	static bool radians = false;
	static bool quaternion = false;
	if (quaternion)
	{
		DrawQuat("Rotation", transform.orientation);
		transform.orientation.Normalise();
	}
	else
	{
		mars::Vec3 ea = Quat::ToEulerAngles(transform.orientation);
		if (!radians)
		{
			Vec3 _ea = Vec3(RadToDeg(ea.x), RadToDeg(ea.y), RadToDeg(ea.z));
			ea = _ea;
		}
		DrawVec3("Rotation", ea);
		if (!radians)
		{
			Vec3 _ea = Vec3(DegToRad(ea.x), DegToRad(ea.y), DegToRad(ea.z));
			ea = _ea;
		}

		transform.orientation = Quat::FromEulerAngles(ea);
		transform.orientation.Normalise();
	}

	//Scale
	DrawVec3("Scale", transform.scale, 1.0f);
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

	if (ImGui::TreeNodeEx("Options", (TreeNodeFlags & ~ImGuiTreeNodeFlags_DefaultOpen), "Options"))
	{
		DrawCheckbox("Radians", radians);
		DrawCheckbox("Quaternion", quaternion);
		DrawCheckbox("Scale Lock", scaleLock);

		ImGui::TreePop();
	}
}
