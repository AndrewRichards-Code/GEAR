#include "gear_core_common.h"

#include "Components.h"
#include "Core/TypeLibrary.h"

using namespace gear;
using namespace scene;
using namespace nlohmann;

template <typename T>
static void LoadReference(const json& references, core::UUID uuid, T& saveData, std::function<void(const json&, T&)> saveDataFunction)
{
	const json& reference = references[uuid.AsString().c_str()];
	bool valid = true;
	valid |= reference["uuid"] == uuid.AsUint64_t();
	valid |= reference["type"]["name"] == typeid(T).name();
	valid |= reference["type"]["hash_code"] == typeid(T).hash_code();
	if (!valid)
	{
		GEAR_ASSERT(ErrorCode::SCENE | ErrorCode::INVALID_VALUE, "UUID, typeid::name() or typeid::hash_code() was incorrect.");
	}
	const json& data = reference["data"];
	saveDataFunction(data, saveData);
}

template <typename T>
static void SaveReference(ordered_json& references, core::UUID uuid, const T& saveData, std::function<void(ordered_json&, const T&)> saveDataFunction)
{
	ordered_json& reference = references[uuid.AsString().c_str()];
	reference["uuid"] = uuid.AsUint64_t();
	reference["type"]["name"] = typeid(T).name();
	reference["type"]["hash_code"] = typeid(T).hash_code();
	ordered_json& data = reference["data"];
	saveDataFunction(data, saveData);
}

bool UUIDComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["UUIDComponent"];
	uuid = core::UUID(component["uuid"]);
	return true;
}
void UUIDComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["UUIDComponent"];
	component["uuid"] = uuid.AsUint64_t();
}

bool NameComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["NameComponent"];
	name = component["name"];
	return true;
}
void NameComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["NameComponent"];
	component["name"] = name;
}

void TransformComponent::Load(const json& component, Transform& transform)
{
	transform.translation.x = component["translation"][0];
	transform.translation.y = component["translation"][1];
	transform.translation.z = component["translation"][2];
	transform.orientation.s = component["orientation"][0];
	transform.orientation.i = component["orientation"][1];
	transform.orientation.j = component["orientation"][2];
	transform.orientation.k = component["orientation"][3];
	transform.scale.x = component["scale"][0];
	transform.scale.y = component["scale"][1];
	transform.scale.z = component["scale"][2];
}
void TransformComponent::Save(ordered_json& component, const Transform& transform)
{
	component["translation"] = { transform.translation.x, transform.translation.y, transform.translation.z };
	component["orientation"] = { transform.orientation.s, transform.orientation.i, transform.orientation.j, transform.orientation.k };
	component["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };
}
bool TransformComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["TransformComponent"];
	Load(component, transform); 
	return true;
}
void TransformComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["TransformComponent"];
	Save(component, transform);
}

bool CameraComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["CameraComponent"];
	if (component.empty())
		return false;

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = component["debugName"];
	cameraCI.device = window->GetDevice();
	TransformComponent::Load(component, cameraCI.transform);
	cameraCI.projectionType = component["projectionType"];
	cameraCI.orthographicsParams.left = component["orthographicsParams"]["left"];
	cameraCI.orthographicsParams.right = component["orthographicsParams"]["right"];
	cameraCI.orthographicsParams.bottom = component["orthographicsParams"]["bottom"];
	cameraCI.orthographicsParams.top = component["orthographicsParams"]["top"];
	cameraCI.orthographicsParams.near = component["orthographicsParams"]["near"];
	cameraCI.orthographicsParams.far = component["orthographicsParams"]["far"];
	cameraCI.perspectiveParams.horizonalFOV = component["perspectiveParams"]["horizonalFOV"];
	cameraCI.perspectiveParams.aspectRatio = component["perspectiveParams"]["aspectRatio"];
	cameraCI.perspectiveParams.zNear = component["perspectiveParams"]["zNear"];
	cameraCI.perspectiveParams.zFar = component["perspectiveParams"]["zFar"];
	cameraCI.flipX = component["flipX"];
	cameraCI.flipY = component["flipY"];

	camera = CreateRef<Camera>(&cameraCI);
	return true;
}
void CameraComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["CameraComponent"];

	Camera::CreateInfo& cameraCI = GetCreateInfo();
	component["debugName"] = cameraCI.debugName;
	TransformComponent::Save(component, cameraCI.transform);
	component["projectionType"] = cameraCI.projectionType;
	component["orthographicsParams"]["left"] = cameraCI.orthographicsParams.left;
	component["orthographicsParams"]["right"] = cameraCI.orthographicsParams.right;
	component["orthographicsParams"]["bottom"] = cameraCI.orthographicsParams.bottom;
	component["orthographicsParams"]["top"] = cameraCI.orthographicsParams.top;
	component["orthographicsParams"]["near"] = cameraCI.orthographicsParams.near;
	component["orthographicsParams"]["far"] = cameraCI.orthographicsParams.far;
	component["perspectiveParams"]["horizonalFOV"] = cameraCI.perspectiveParams.horizonalFOV;
	component["perspectiveParams"]["aspectRatio"] = cameraCI.perspectiveParams.aspectRatio;
	component["perspectiveParams"]["zNear"] = cameraCI.perspectiveParams.zNear;
	component["perspectiveParams"]["zFar"] = cameraCI.perspectiveParams.zFar;
	component["flipX"] = cameraCI.flipX;
	component["flipY"] = cameraCI.flipY;
}

bool LightComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["LightComponent"];
	if (component.empty())
		return false;

	Light::CreateInfo lightCI;
	lightCI.debugName = component["debugName"];
	lightCI.device = window->GetDevice();
	lightCI.type = component["type"];
	lightCI.colour.r = component["colour"][0];
	lightCI.colour.g = component["colour"][1];
	lightCI.colour.b = component["colour"][2];
	lightCI.colour.a = component["colour"][3];
	TransformComponent::Load(component, lightCI.transform);

	light = CreateRef<Light>(&lightCI);
	return true;
}
void LightComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["LightComponent"];

	Light::CreateInfo& lightCI = GetCreateInfo();
	component["debugName"] = lightCI.debugName;
	component["type"] = lightCI.type;
	component["colour"][0] = { lightCI.colour.r, lightCI.colour.g, lightCI.colour.b, lightCI.colour.a };
	TransformComponent::Save(component, lightCI.transform);
}

bool SkyboxComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["SkyboxComponent"];
	if (component.empty())
		return false;

	Skybox::CreateInfo skyboxCI;
	skyboxCI.debugName = component["debugName"];
	skyboxCI.device = window->GetDevice();
	for(const auto& filepath : component["filepaths"])
		skyboxCI.filepaths.push_back(filepath);
	skyboxCI.generatedCubemapSize = component["generatedCubemapSize"];
	TransformComponent::Load(component, skyboxCI.transform);
	skyboxCI.exposure = component["exposure"];
	skyboxCI.gammaSpace = component["gammaSpace"];

	skybox = CreateRef<Skybox>(&skyboxCI);
	return true;
}
void SkyboxComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& component = entity["SkyboxComponent"];

	Skybox::CreateInfo& skyboxCI = GetCreateInfo();
	component["debugName"] = skyboxCI.debugName;
	for (const auto& filepath : skyboxCI.filepaths)
		component["filepaths"].push_back(filepath);
	component["generatedCubemapSize"] = skyboxCI.generatedCubemapSize;
	TransformComponent::Save(component, skyboxCI.transform);
	component["exposure"] = skyboxCI.exposure;
	component["gammaSpace"] = skyboxCI.gammaSpace;
}

static core::TypeLibrary<FontLibrary::Font> s_Fonts;
bool TextComponent::Load(json& references, json& entity, const Ref<graphics::Window>& window)
{
	const json& component = entity["TextComponent"];
	if (component.empty())
		return false;

	Text::CreateInfo textCI;
	textCI.device = window->GetDevice();
	textCI.viewportHeight = window->GetWidth();
	textCI.viewportWidth = window->GetHeight();
	text = CreateRef<Text>(&textCI);

	for (auto& line : component["lines"])
	{
		Ref<FontLibrary::Font> font;
		core::UUID uuid = core::UUID(line["font"]);
		if (s_Fonts.Has(uuid))
			font = s_Fonts.Get(uuid);
		else
		{
			FontLibrary::LoadInfo loadInfo;
			LoadReference<FontLibrary::LoadInfo>(references, uuid, loadInfo,
				[](const json& data, FontLibrary::LoadInfo& loadInfo)
				{
					loadInfo.GI.filepath = data["GI"]["filepath"];
					loadInfo.GI.fontHeightPx = data["GI"]["fontHeightPx"];
					loadInfo.GI.generatedTextureSize = data["GI"]["generatedTextureSize"];
					loadInfo.GI.savePNGandBINfiles = data["GI"]["savePNGandBINfiles"];
					loadInfo.regenerateTextureAtlas = data["regenerateTextureAtlas"];
				});

			Ref<FontLibrary> fontLib = CreateRef<FontLibrary>();
			font = fontLib->LoadFont(&loadInfo);
		}


		text->AddLine(
			font,
			line["text"],
			{ (uint32_t)line["position"][0], (uint32_t)line["position"][1] },
			{ line["colour"][0], line["colour"][1], line["colour"][2], line["colour"][3] },
			{ line["backgroundColour"][0], line["backgroundColour"][1], line["backgroundColour"][2], line["backgroundColour"][3] }
		);
	}
	return true;
}
void TextComponent::Save(ordered_json& references, ordered_json& entity)
{
	ordered_json& lines = entity["TextComponent"]["lines"];

	size_t idx = 0;
	for (auto& line : text->GetLines())
	{
		core::UUID uuid;
		if (!s_Fonts.FindUUID(line.font, uuid))
		{
			s_Fonts.Insert(uuid, line.font);

			SaveReference<FontLibrary::LoadInfo>(references, uuid, line.font->loadInfo,
				[](ordered_json& data, const FontLibrary::LoadInfo& loadInfo)
				{
					data["GI"]["filepath"] = loadInfo.GI.filepath;
					data["GI"]["fontHeightPx"] = loadInfo.GI.fontHeightPx;
					data["GI"]["generatedTextureSize"] = loadInfo.GI.generatedTextureSize;
					data["GI"]["savePNGandBINfiles"] = loadInfo.GI.savePNGandBINfiles;
					data["regenerateTextureAtlas"] = loadInfo.regenerateTextureAtlas;
				});
		}

		ordered_json& _line = lines[std::to_string(idx).c_str()];
		_line["font"] = uuid.AsUint64_t();
		_line["text"] = line.text;
		_line["position"] = { line.position.x, line.position.y };
		_line["colour"] = { line.colour.r, line.colour.g, line.colour.b, line.colour.a };
		_line["backgroundColour"] = { line.backgroundColour.r, line.backgroundColour.g, line.backgroundColour.b, line.backgroundColour.a };
	}
}
