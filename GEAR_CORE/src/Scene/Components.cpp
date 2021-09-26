#include "gear_core_common.h"

#include "Components.h"
#include "Core/TypeLibrary.h"

using namespace gear;
using namespace scene;
using namespace nlohmann;

bool UUIDComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["UUIDComponent"];
	uuid = core::UUID(child["uuid"]);
	return true;
}
void UUIDComponent::Save(ordered_json& parent)
{
	ordered_json& child = parent["UUIDComponent"];
	child["uuid"] = uuid.AsUint64_t();
}

bool NameComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["NameComponent"];
	name = child["name"];
	return true;
}
void NameComponent::Save(ordered_json& parent)
{
	ordered_json& child = parent["NameComponent"];
	child["name"] = name;
}

void TransformComponent::Load(const json& parent, Transform& transform)
{
	transform.translation.x = parent["translation"][0];
	transform.translation.y = parent["translation"][1];
	transform.translation.z = parent["translation"][2];
	transform.orientation.s = parent["orientation"][0];
	transform.orientation.i = parent["orientation"][1];
	transform.orientation.j = parent["orientation"][2];
	transform.orientation.k = parent["orientation"][3];
	transform.scale.x = parent["scale"][0];
	transform.scale.y = parent["scale"][1];
	transform.scale.z = parent["scale"][2];
}
void TransformComponent::Save(ordered_json& parent, const Transform& transform)
{
	parent["translation"] = { transform.translation.x, transform.translation.y, transform.translation.z };
	parent["orientation"] = { transform.orientation.s, transform.orientation.i, transform.orientation.j, transform.orientation.k };
	parent["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };
}
bool TransformComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["TransformComponent"];
	Load(child, transform); 
	return true;
}
void TransformComponent::Save(ordered_json& parent)
{
	ordered_json& child = parent["TransformComponent"];
	Save(child, transform);
}

bool CameraComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["CameraComponent"];
	if (child.empty())
		return false;

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = child["debugName"];
	cameraCI.device = window->GetDevice();
	TransformComponent::Load(child, cameraCI.transform);
	cameraCI.projectionType = child["projectionType"];
	cameraCI.orthographicsParams.left = child["orthographicsParams"]["left"];
	cameraCI.orthographicsParams.right = child["orthographicsParams"]["right"];
	cameraCI.orthographicsParams.bottom = child["orthographicsParams"]["bottom"];
	cameraCI.orthographicsParams.top = child["orthographicsParams"]["top"];
	cameraCI.orthographicsParams.near = child["orthographicsParams"]["near"];
	cameraCI.orthographicsParams.far = child["orthographicsParams"]["far"];
	cameraCI.perspectiveParams.horizonalFOV = child["perspectiveParams"]["horizonalFOV"];
	cameraCI.perspectiveParams.aspectRatio = child["perspectiveParams"]["aspectRatio"];
	cameraCI.perspectiveParams.zNear = child["perspectiveParams"]["zNear"];
	cameraCI.perspectiveParams.zFar = child["perspectiveParams"]["zFar"];
	cameraCI.flipX = child["flipX"];
	cameraCI.flipY = child["flipY"];

	camera = CreateRef<Camera>(&cameraCI);
	return true;
}
void CameraComponent::Save(ordered_json& parent)
{
	ordered_json& child = parent["CameraComponent"];

	Camera::CreateInfo& cameraCI = GetCreateInfo();
	child["debugName"] = cameraCI.debugName;
	TransformComponent::Save(child, cameraCI.transform);
	child["projectionType"] = cameraCI.projectionType;
	child["orthographicsParams"]["left"] = cameraCI.orthographicsParams.left;
	child["orthographicsParams"]["right"] = cameraCI.orthographicsParams.right;
	child["orthographicsParams"]["bottom"] = cameraCI.orthographicsParams.bottom;
	child["orthographicsParams"]["top"] = cameraCI.orthographicsParams.top;
	child["orthographicsParams"]["near"] = cameraCI.orthographicsParams.near;
	child["orthographicsParams"]["far"] = cameraCI.orthographicsParams.far;
	child["perspectiveParams"]["horizonalFOV"] = cameraCI.perspectiveParams.horizonalFOV;
	child["perspectiveParams"]["aspectRatio"] = cameraCI.perspectiveParams.aspectRatio;
	child["perspectiveParams"]["zNear"] = cameraCI.perspectiveParams.zNear;
	child["perspectiveParams"]["zFar"] = cameraCI.perspectiveParams.zFar;
	child["flipX"] = cameraCI.flipX;
	child["flipY"] = cameraCI.flipY;
}

bool LightComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["LightComponent"];
	if (child.empty())
		return false;

	Light::CreateInfo lightCI;
	lightCI.debugName = child["debugName"];
	lightCI.device = window->GetDevice();
	lightCI.type = child["type"];
	lightCI.colour.r = child["colour"][0];
	lightCI.colour.g = child["colour"][1];
	lightCI.colour.b = child["colour"][2];
	lightCI.colour.a = child["colour"][3];
	TransformComponent::Load(child, lightCI.transform);

	light = CreateRef<Light>(&lightCI);
	return true;
}
void LightComponent::Save(ordered_json& parent)
{
	ordered_json& child = parent["LightComponent"];

	Light::CreateInfo& lightCI = GetCreateInfo();
	child["debugName"] = lightCI.debugName;
	child["type"] = lightCI.type;
	child["colour"][0] = { lightCI.colour.r, lightCI.colour.g, lightCI.colour.b, lightCI.colour.a };
	TransformComponent::Save(child, lightCI.transform);
}

static core::TypeLibrary<FontLibrary::Font> s_Fonts;
bool TextComponent::Load(json& parent, const Ref<graphics::Window>& window)
{
	const json& child = parent["TextComponent"];
	if (child.empty())
		return false;

	Text::CreateInfo textCI;
	textCI.device = window->GetDevice();
	textCI.viewportHeight = window->GetWidth();
	textCI.viewportWidth = window->GetHeight();
	text = CreateRef<Text>(&textCI);

	for (auto& line : child["lines"])
	{
		text->AddLine(
			s_Fonts.Find(core::UUID(line["font"])),
			line["text"],
			{ (uint32_t)line["position"][0], (uint32_t)line["position"][1] },
			{ line["colour"][0], line["colour"][1], line["colour"][2], line["colour"][3] },
			{ line["backgroundColour"][0], line["backgroundColour"][1], line["backgroundColour"][2], line["backgroundColour"][3] }
		);
	}
	return true;
}
void TextComponent::Save(ordered_json& parent)
{
	ordered_json& lines = parent["TextComponent"]["lines"];

	size_t idx = 0;
	for (auto& line : text->GetLines())
	{
		core::UUID uuid;
		s_Fonts.Insert(uuid, line.font);

		ordered_json& _line = lines[std::to_string(idx).c_str()];
		_line["font"] = uuid.AsUint64_t();
		_line["text"] = line.text;
		_line["position"] = { line.position.x, line.position.y };
		_line["colour"] = { line.colour.r, line.colour.g, line.colour.b, line.colour.a };
		_line["backgroundColour"] = { line.backgroundColour.r, line.backgroundColour.g, line.backgroundColour.b, line.backgroundColour.a };
	}
}
