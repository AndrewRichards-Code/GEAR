#include "gear_core_common.h"

#include "Components.h"
#include "Scene.h"

using namespace gear;
using namespace scene;
using namespace objects;
using namespace nlohmann;

//Helper
template<typename T>
bool FindByValue(std::map<core::UUID, Ref<T>>& map, Ref<T> value, core::UUID outUUID)
{
	for (const auto& entry : map)
	{
		if (entry.second == value)
		{
			outUUID = entry.first;
			return true;
		}
	}
	return false;
}

//References

template <typename T>
static void LoadReference(const json& references, core::UUID uuid, T& saveData, std::function<void(const json&, T&)> saveDataFunction)
{
	const json& reference = references[uuid.AsString().c_str()];
	bool valid = true;
	valid |= (reference["uuid"] == uuid.AsUint64_t());
	valid |= (std::string(reference["type"]["name"]).compare(typeid(T).name()) == 0);
	valid |= (reference["type"]["hash_code"] == typeid(T).hash_code());
	if (!valid)
	{
		GEAR_ASSERT(ErrorCode::SCENE | ErrorCode::INVALID_VALUE, "UUID, typeid::name() or typeid::hash_code() was incorrect.");
	}
	const json& data = reference["data"];
	saveDataFunction(data, saveData);
}
template <typename T>
static void SaveReference(json& references, core::UUID uuid, const T& saveData, std::function<void(json&, const T&)> saveDataFunction)
{
	json& reference = references[uuid.AsString().c_str()];
	reference["uuid"] = uuid.AsUint64_t();
	reference["type"]["name"] = typeid(T).name();
	reference["type"]["hash_code"] = typeid(T).hash_code();
	json& data = reference["data"];
	saveDataFunction(data, saveData);
}

//Components

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(UUIDComponent);
void UUIDComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["UUIDComponent"];
	uuid = core::UUID(component["uuid"]);
}
void UUIDComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["UUIDComponent"];
	component["uuid"] = uuid.AsUint64_t();
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(NameComponent);
void NameComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["NameComponent"];
	name = component["name"];
}
void NameComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["NameComponent"];
	component["name"] = name;
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(TransformComponent);

void TransformComponent::Load(const json& component, Transform& transform)
{
	transform.translation.x = component["transform"]["translation"][0];
	transform.translation.y = component["transform"]["translation"][1];
	transform.translation.z = component["transform"]["translation"][2];
	transform.orientation.s = component["transform"]["orientation"][0];
	transform.orientation.i = component["transform"]["orientation"][1];
	transform.orientation.j = component["transform"]["orientation"][2];
	transform.orientation.k = component["transform"]["orientation"][3];
	transform.scale.x = component["transform"]["scale"][0];
	transform.scale.y = component["transform"]["scale"][1];
	transform.scale.z = component["transform"]["scale"][2];
}
void TransformComponent::Save(json& component, const Transform& transform)
{
	component["transform"]["translation"] = { transform.translation.x, transform.translation.y, transform.translation.z };
	component["transform"]["orientation"] = { transform.orientation.s, transform.orientation.i, transform.orientation.j, transform.orientation.k };
	component["transform"]["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };
}
void TransformComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["TransformComponent"];
	Load(component, transform);
}
void TransformComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["TransformComponent"];
	Save(component, transform);
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(CameraComponent);
void CameraComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["CameraComponent"];

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = component["debugName"];
	cameraCI.device = lsp.window->GetDevice();
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
}
void CameraComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["CameraComponent"];

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

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(LightComponent);
void LightComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["LightComponent"];

	Light::CreateInfo lightCI;
	lightCI.debugName = component["debugName"];
	lightCI.device = lsp.window->GetDevice();
	lightCI.type = component["type"];
	lightCI.colour.r = component["colour"][0];
	lightCI.colour.g = component["colour"][1];
	lightCI.colour.b = component["colour"][2];
	lightCI.colour.a = component["colour"][3];
	TransformComponent::Load(component, lightCI.transform);

	light = CreateRef<Light>(&lightCI);
}
void LightComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["LightComponent"];

	Light::CreateInfo& lightCI = GetCreateInfo();
	component["debugName"] = lightCI.debugName;
	component["type"] = lightCI.type;
	component["colour"] = { lightCI.colour.r, lightCI.colour.g, lightCI.colour.b, lightCI.colour.a };
	TransformComponent::Save(component, lightCI.transform);
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(ModelComponent);
void ModelComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["ModelComponent"];

	Ref<Mesh> mesh;
	core::UUID uuid = core::UUID(component["mesh"]["uuid"]);
	if (lsp.scene->s_Meshes.find(uuid) != lsp.scene->s_Meshes.end())
		mesh = lsp.scene->s_Meshes.at(uuid);
	else
	{
		Mesh::CreateInfo meshCI;
		LoadReference<Mesh::CreateInfo>(lsp.references, uuid, meshCI,
			[&](const json& data, Mesh::CreateInfo& CI)
			{
				CI.debugName = data["debugName"];
				CI.device = lsp.window->GetDevice();
				CI.filepath = data["filepath"];
			});

		mesh = CreateRef<Mesh>(&meshCI);
	}

	Ref<Material> material;
	size_t i = 0;
	for (auto& materialUUID : component["mesh"]["material"])
	{
		core::UUID uuid = core::UUID(materialUUID);
		if (lsp.scene->s_Materials.find(uuid) != lsp.scene->s_Materials.end())
			material = lsp.scene->s_Materials.at(uuid);
		else
		{
			Material::CreateInfo materialCI;
			LoadReference<Material::CreateInfo>(lsp.references, uuid, materialCI,
				[&](const json& data, Material::CreateInfo& CI)
				{
					CI.debugName = data["debugName"];
					CI.device = lsp.window->GetDevice();
					CI.filepath = data["filepath"];
				});
			
			material = CreateRef<Material>(&materialCI);
		}
		mesh->SetOverrideMaterial(i, material);
		i++;
	}

	Model::CreateInfo modelCI;
	modelCI.debugName = component["debugName"];
	modelCI.device = lsp.window->GetDevice();
	modelCI.pMesh = mesh;
	TransformComponent::Load(component, modelCI.transform);
	modelCI.renderPipelineName = component["renderPipelineName"];

	model = CreateRef<Model>(&modelCI);
}
void ModelComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["ModelComponent"];
	
	Model::CreateInfo& modelCI = GetCreateInfo();
	Mesh::CreateInfo& meshCI = modelCI.pMesh->m_CI;

	core::UUID uuid;
	if (!FindByValue(lsp.scene->s_Meshes, modelCI.pMesh, uuid))
		lsp.scene->s_Meshes.insert({ uuid, modelCI.pMesh });

	SaveReference<Mesh::CreateInfo>(lsp.references, uuid, meshCI,
		[](json& data, const Mesh::CreateInfo& CI)
		{
			data["debugName"] = CI.debugName;
			data["filepath"] = CI.filepath;
		});

	component["debugName"] = modelCI.debugName;
	component["mesh"]["uuid"] = uuid.AsUint64_t();
	TransformComponent::Save(component, modelCI.transform);
	component["renderPipelineName"] = modelCI.renderPipelineName;

	for (auto& material : modelCI.pMesh->GetMaterials())
	{
		core::UUID uuid;
		if (!FindByValue(lsp.scene->s_Materials, material, uuid))
			lsp.scene->s_Materials.insert({ uuid, material });

		Material::CreateInfo& materialCI = material->m_CI;
		if (materialCI.filepath.empty())
		{
			materialCI.filepath = std::string("res/assets/") + materialCI.debugName + std::string(".gaf");
			core::AssetFile assetFile(materialCI.filepath);
			material->SaveToAssetFile(assetFile);
			assetFile.Save();
		}
		SaveReference<Material::CreateInfo>(lsp.references, uuid, materialCI,
			[](json& data, const Material::CreateInfo& CI)
			{
				data["debugName"] = CI.debugName;
				data["filepath"] = CI.filepath;
			});
		
		component["mesh"]["material"].push_back(uuid.AsUint64_t());
	}
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(SkyboxComponent);
void SkyboxComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["SkyboxComponent"];

	Skybox::CreateInfo skyboxCI;
	skyboxCI.debugName = component["debugName"];
	skyboxCI.device = lsp.window->GetDevice();
	for(const auto& filepath : component["filepaths"])
		skyboxCI.filepaths.push_back(filepath);
	skyboxCI.generatedCubemapSize = component["generatedCubemapSize"];
	TransformComponent::Load(component, skyboxCI.transform);
	skyboxCI.exposure = component["exposure"];
	skyboxCI.gammaSpace = component["gammaSpace"];

	skybox = CreateRef<Skybox>(&skyboxCI);
}
void SkyboxComponent::Save(LoadSaveParameters& lsp)
{
	json& component = lsp.entity["SkyboxComponent"];

	Skybox::CreateInfo& skyboxCI = GetCreateInfo();
	component["debugName"] = skyboxCI.debugName;
	for (const auto& filepath : skyboxCI.filepaths)
		component["filepaths"].push_back(filepath);
	component["generatedCubemapSize"] = skyboxCI.generatedCubemapSize;
	TransformComponent::Save(component, skyboxCI.transform);
	component["exposure"] = skyboxCI.exposure;
	component["gammaSpace"] = skyboxCI.gammaSpace;
}

GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(TextComponent);
void TextComponent::Load(LoadSaveParameters& lsp)
{
	const json& component = lsp.entity["TextComponent"];

	Text::CreateInfo textCI;
	textCI.device = lsp.window->GetDevice();
	textCI.viewportHeight = lsp.window->GetWidth();
	textCI.viewportWidth = lsp.window->GetHeight();
	text = CreateRef<Text>(&textCI);

	for (auto& line : component["lines"])
	{
		Ref<FontLibrary::Font> font;
		core::UUID uuid = core::UUID(line["font"]);
		if (lsp.scene->s_Fonts.find(uuid) != lsp.scene->s_Fonts.end())
			font = lsp.scene->s_Fonts.at(uuid);
		else
		{
			FontLibrary::LoadInfo loadInfo;
			LoadReference<FontLibrary::LoadInfo>(lsp.references, uuid, loadInfo,
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
}
void TextComponent::Save(LoadSaveParameters& lsp)
{
	json& lines = lsp.entity["TextComponent"]["lines"];

	size_t idx = 0;
	for (auto& line : text->GetLines())
	{
		core::UUID uuid;
		if (!FindByValue(lsp.scene->s_Fonts, line.font, uuid))
		{
			lsp.scene->s_Fonts.insert({ uuid, line.font });

			SaveReference<FontLibrary::LoadInfo>(lsp.references, uuid, line.font->loadInfo,
				[](json& data, const FontLibrary::LoadInfo& loadInfo)
				{
					data["GI"]["filepath"] = loadInfo.GI.filepath;
					data["GI"]["fontHeightPx"] = loadInfo.GI.fontHeightPx;
					data["GI"]["generatedTextureSize"] = loadInfo.GI.generatedTextureSize;
					data["GI"]["savePNGandBINfiles"] = loadInfo.GI.savePNGandBINfiles;
					data["regenerateTextureAtlas"] = loadInfo.regenerateTextureAtlas;
				});
		}

		json& _line = lines[std::to_string(idx).c_str()];
		_line["font"] = uuid.AsUint64_t();
		_line["text"] = line.text;
		_line["position"] = { line.position.x, line.position.y };
		_line["colour"] = { line.colour.r, line.colour.g, line.colour.b, line.colour.a };
		_line["backgroundColour"] = { line.backgroundColour.r, line.backgroundColour.g, line.backgroundColour.b, line.backgroundColour.a };
	}
}
