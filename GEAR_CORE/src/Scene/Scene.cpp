#include "gear_core_common.h"
#include "Scene.h"
#include "Entity.h"
#include "NativeScriptManager.h"
#include "INativeScript.h"

#include "Core/Timer.h"
#include "Graphics/Renderer.h"

using namespace arc;
using namespace gear;
using namespace scene;

static DynamicLibrary::LibraryHandle s_NativeScriptLibrary = 0;

Scene::Scene(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	LoadNativeScriptLibrary();
}

Scene::~Scene()
{
	UnloadNativeScriptLibrary();
}

Entity Scene::CreateEntity()
{
	Entity::CreateInfo entityCI = { this };
	Entity entity(&entityCI);

	entity.AddComponent<NameComponent>();
	entity.AddComponent<TransformComponent>();
	
	return entity;
}

void Scene::DestroyEntity(Entity entity)
{
	m_Registry.destroy(entity.m_Entity);
}

void Scene::OnUpdate(Ref<graphics::Renderer>& renderer, core::Timer& timer)
{
	if (m_Playing)
	{
		auto& vNativeScriptComponents = m_Registry.view<NativeScriptComponent>();
		for (auto& entity : vNativeScriptComponents)
		{
			NativeScriptComponent& nativeScriptComponent = vNativeScriptComponents.get<NativeScriptComponent>(entity);
			INativeScript*& nativeScript = nativeScriptComponent.pNativeScript;
			if (!nativeScript && s_NativeScriptLibrary)
			{
				nativeScript = NativeScriptManager::LoadScript(s_NativeScriptLibrary, nativeScriptComponent.nativeScriptName);
				if (nativeScript)
				{
					nativeScript->SetComponents(nativeScriptComponent);
					nativeScript->OnCreate();
				}
			}

			if (nativeScript)
			{
				nativeScript->OnUpdate(timer);
			}
		}
	}

	auto& vCameraComponents = m_Registry.view<CameraComponent>();
	if (!vCameraComponents.empty())
	{
		for (auto& entity : vCameraComponents)
		{
			renderer->SubmitCamera(vCameraComponents.get<CameraComponent>(entity));
		}
	}
	else
	{
		renderer->SubmitCamera(nullptr);
	}

	std::vector<Ref<Light>> lights;
	auto& vLightComponents = m_Registry.view<LightComponent>();
	for (auto& entity : vLightComponents)
	{
		lights.push_back(vLightComponents.get<LightComponent>(entity));	
	}
	renderer->SubmitLights(lights);

	auto& vModelComponents = m_Registry.view<ModelComponent>();
	for (auto& entity : vModelComponents)
	{
		renderer->SubmitModel(vModelComponents.get<ModelComponent>(entity));
	}

	auto& vSkyboxComponent = m_Registry.view<SkyboxComponent>();
	for (auto& entity : vSkyboxComponent)
	{
		renderer->SubmitSkybox(vSkyboxComponent.get<SkyboxComponent>(entity));
	}

	auto& vTextComponent = m_Registry.view<TextComponent>();
	for (auto& entity : vTextComponent)
	{
		const Ref<Text>& text = vTextComponent.get<TextComponent>(entity).text;
		renderer->SubmitTextCamera(text->GetCamera());

		for (auto& line : text->GetLines())
		{
			renderer->SubmitTextLine(line.model);
		}
	}
}

entt::registry& Scene::GetRegistry()
{
	return m_Registry;
}

void Scene::LoadNativeScriptLibrary()
{
	if (!s_NativeScriptLibrary)
	{
		NativeScriptManager::Build(m_CI.nativeScriptDir);
		s_NativeScriptLibrary = NativeScriptManager::Load();
	}
}

void Scene::UnloadNativeScriptLibrary()
{
	auto& vNativeScriptComponents = m_Registry.view<NativeScriptComponent>();
	for (auto& entity : vNativeScriptComponents)
	{
		NativeScriptComponent& nativeScriptComponent = vNativeScriptComponents.get<NativeScriptComponent>(entity);
		INativeScript*& nativeScript = nativeScriptComponent.pNativeScript;
		if (nativeScript)
		{
			nativeScript->OnDestroy();
			NativeScriptManager::UnloadScript(s_NativeScriptLibrary, nativeScriptComponent.nativeScriptName, nativeScript);
		}
	}

	if (s_NativeScriptLibrary)
		NativeScriptManager::Unload(s_NativeScriptLibrary);
}

void Scene::LoadFromFile()
{
}

void Scene::SaveToFile()
{
	using namespace nlohmann;

	ordered_json scene_gsf_json;
	ordered_json& scene = scene_gsf_json["scene"];
	scene["debugName"] = m_CI.debugName;

	ordered_json& cameras = scene_gsf_json["scene"]["cameras"];
	auto& vCameraComponents = m_Registry.view<CameraComponent>();
	for (auto& entity : vCameraComponents)
	{
		auto& cameraCI = vCameraComponents.get<CameraComponent>(entity).GetCreateInfo();

		ordered_json& camera = cameras[cameraCI.debugName.c_str()];
		camera["debugName"] = cameraCI.debugName;
		camera["transform"]["translation"] = { cameraCI.transform.translation.x, cameraCI.transform.translation.y, cameraCI.transform.translation.z };
		camera["transform"]["orientation"] = { cameraCI.transform.orientation.s, cameraCI.transform.orientation.i, cameraCI.transform.orientation.j, cameraCI.transform.orientation.k };
		camera["transform"]["scale"] = { cameraCI.transform.scale.x, cameraCI.transform.scale.y, cameraCI.transform.scale.z };
		camera["projectionType"] = cameraCI.projectionType;
		camera["orthographicsParams"]["left"] = cameraCI.orthographicsParams.left;
		camera["orthographicsParams"]["right"] = cameraCI.orthographicsParams.right;
		camera["orthographicsParams"]["bottom"] = cameraCI.orthographicsParams.bottom;
		camera["orthographicsParams"]["top"] = cameraCI.orthographicsParams.top;
		camera["orthographicsParams"]["near"] = cameraCI.orthographicsParams.near;
		camera["orthographicsParams"]["far"] = cameraCI.orthographicsParams.far;
		camera["perspectiveParams"]["horizonalFOV"] = cameraCI.perspectiveParams.horizonalFOV;
		camera["perspectiveParams"]["aspectRatio"] = cameraCI.perspectiveParams.aspectRatio;
		camera["perspectiveParams"]["zNear"] = cameraCI.perspectiveParams.zNear;
		camera["perspectiveParams"]["zFar"] = cameraCI.perspectiveParams.zFar;
		camera["flipX"] = cameraCI.flipX;
		camera["flipY"] = cameraCI.flipY;
	}

	ordered_json& lights = scene_gsf_json["scene"]["lights"];
	auto& vLightComponents = m_Registry.view<LightComponent>();
	for (auto& entity : vLightComponents)
	{
		auto& lightCI = vLightComponents.get<LightComponent>(entity).GetCreateInfo();

		ordered_json& light = lights[lightCI.debugName.c_str()];
		light["debugName"] = lightCI.debugName;
		light["type"] = lightCI.type;
		light["colour"] = { lightCI.colour.r, lightCI.colour.g, lightCI.colour.b, lightCI.colour.a };
		light["transform"]["translation"] = { lightCI.transform.translation.x, lightCI.transform.translation.y, lightCI.transform.translation.z };
		light["transform"]["orientation"] = { lightCI.transform.orientation.s, lightCI.transform.orientation.i, lightCI.transform.orientation.j, lightCI.transform.orientation.k };
		light["transform"]["scale"] = { lightCI.transform.scale.x, lightCI.transform.scale.y, lightCI.transform.scale.z };
	}

	ordered_json& models = scene_gsf_json["scene"]["models"];
	auto& vModelComponents = m_Registry.view<ModelComponent>();
	for (auto& entity : vModelComponents)
	{
		auto& modelCI = vModelComponents.get<ModelComponent>(entity).GetCreateInfo();
		auto& meshCI = modelCI.pMesh->m_CI;

		ordered_json& model = models[modelCI.debugName.c_str()];
		model["debugName"] = modelCI.debugName;

		ordered_json& mesh = model["mesh"];
		mesh["debugName"] = meshCI.debugName;
		mesh["filepath"] = meshCI.filepath;

		ordered_json& materials = model["materials"];
		for (auto& material : modelCI.pMesh->GetMaterials())
		{
			auto& materialCI = material->m_CI;

			ordered_json& material = materials[materialCI.debugName.c_str()];
			material["debugName"] = materialCI.debugName;

			ordered_json& textures = material["textures"];
			for (auto& pbrTexture : materialCI.pbrTextures)
			{
				auto& textureCI = pbrTexture.second->GetCreateInfo();

				ordered_json& texture = textures[textureCI.debugName.c_str()];
				texture["pbrType"] = pbrTexture.first;
				texture["debugName"] = textureCI.debugName;
				texture["arrayLayers"] = textureCI.arrayLayers;
				texture["mipLevels"] = textureCI.mipLevels;
				texture["type"] = textureCI.type;
				texture["format"] = textureCI.format;
				texture["samples"] = textureCI.samples;
				texture["usage"] = textureCI.usage;
				texture["generateMipMaps"] = textureCI.generateMipMaps;
			}

			material["fresnel"] = { materialCI.pbrConstants.fresnel.r, materialCI.pbrConstants.fresnel.g, materialCI.pbrConstants.fresnel.b, materialCI.pbrConstants.fresnel.a };
			material["albedo"] = { materialCI.pbrConstants.albedo.r, materialCI.pbrConstants.albedo.g, materialCI.pbrConstants.albedo.b, materialCI.pbrConstants.albedo.a };
			material["metallic"] = materialCI.pbrConstants.metallic;
			material["roughness"] = materialCI.pbrConstants.roughness;
			material["ambientOcclusion"] = materialCI.pbrConstants.ambientOcclusion;
			material["emissive"] = { materialCI.pbrConstants.emissive.r, materialCI.pbrConstants.emissive.g, materialCI.pbrConstants.emissive.b, materialCI.pbrConstants.emissive.a };

		}

		model["materialTextureScaling"] = { modelCI.materialTextureScaling.x, modelCI.materialTextureScaling.y };
		model["transform"]["translation"] = { modelCI.transform.translation.x, modelCI.transform.translation.y, modelCI.transform.translation.z };
		model["transform"]["orientation"] = { modelCI.transform.orientation.s, modelCI.transform.orientation.i, modelCI.transform.orientation.j, modelCI.transform.orientation.k };
		model["transform"]["scale"] = { modelCI.transform.scale.x, modelCI.transform.scale.y, modelCI.transform.scale.z };
		model["renderPipelineName"] = modelCI.renderPipelineName;
	}

	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path fullSaveFilepath = cwd / std::filesystem::path(m_CI.filepath);
	std::filesystem::path fullSaveFileDir = cwd / std::filesystem::path(m_CI.filepath).remove_filename();
	
	if (!std::filesystem::exists(fullSaveFileDir))
		std::filesystem::create_directory(fullSaveFileDir);

	std::ofstream saveFile(fullSaveFilepath.string(), std::ios::binary);
	if (!saveFile.is_open())
	{
		GEAR_WARN(ErrorCode::SCENE | ErrorCode::NO_FILE, "Can not save to file: %s", fullSaveFilepath.string().c_str());
		return;
	}
	else
	{
		saveFile << std::setw(4) << scene;
	}
}
