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

	entity.AddComponent<UUIDComponent>();
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
	if (!vSkyboxComponent.empty())
	{
		for (auto& entity : vSkyboxComponent)
		{
			renderer->SubmitSkybox(vSkyboxComponent.get<SkyboxComponent>(entity));
		}
	}
	else
	{
		renderer->SubmitSkybox(nullptr);
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

void Scene::LoadEntity(nlohmann::json& references, nlohmann::json& entity, Entity entityID, const Ref<graphics::Window>& window)
{
	if (!entityID.HasComponent<UUIDComponent>())
	{
		entityID.AddComponent<UUIDComponent>().Load(references, entity, window);
	}
	if (!entityID.HasComponent<NameComponent>())
	{
		entityID.AddComponent<NameComponent>().Load(references, entity, window);
	}
	if (!entityID.HasComponent<TransformComponent>())
	{
		entityID.AddComponent<TransformComponent>().Load(references, entity, window);
	}
	if (!entityID.HasComponent<CameraComponent>())
	{
		if (!entityID.AddComponent<CameraComponent>().Load(references, entity, window))
			entityID.RemoveComponent<CameraComponent>();
	}
	if (!entityID.HasComponent<LightComponent>())
	{
		if (!entityID.AddComponent<LightComponent>().Load(references, entity, window))
			entityID.RemoveComponent<LightComponent>();
	}
	if (!entityID.HasComponent<SkyboxComponent>())
	{
		if (!entityID.AddComponent<SkyboxComponent>().Load(references, entity, window))
			entityID.RemoveComponent<SkyboxComponent>();
	}
}

void Scene::LoadFromFile(const std::string& filepath, const Ref<graphics::Window>& window)
{
	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path fullSaveFilepath = cwd / std::filesystem::path(filepath);

	if (filepath.find(".gsf") == std::string::npos)
		fullSaveFilepath += ".gsf";

	nlohmann::json scene_gsf_json;
	std::ifstream openFile(fullSaveFilepath, std::ios::binary);
	if (openFile.is_open())
	{
		openFile >> scene_gsf_json;
	}
	else
	{
		GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::NO_FILE, "Unable to open %s.", fullSaveFilepath.c_str());
		return;
	}

	if (scene_gsf_json.empty())
	{
		GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::LOAD_FAILED, "%s is not valid.", fullSaveFilepath.c_str());
		return;
	}

	nlohmann::json& scene = scene_gsf_json;
	m_CI.debugName = scene["debugName"];
	m_UUID = core::UUID(scene["uuid"]);

	nlohmann::json& entities = scene["entities"];
	nlohmann::json& references = scene["references"];
	for (auto& entity_json : entities)
	{
		Entity::CreateInfo entityCI = { this };
		Entity entity(&entityCI);

		LoadEntity(references, entity_json, entity, window);
	}

	m_Filepath = filepath;
}

void Scene::SaveEntity(nlohmann::ordered_json& references, nlohmann::ordered_json& entity, Entity entityID)
{
	if (entityID.HasComponent<UUIDComponent>())
	{
		entityID.GetComponent<UUIDComponent>().Save(references, entity);
	}
	if (entityID.HasComponent<NameComponent>())
	{
		entityID.GetComponent<NameComponent>().Save(references, entity);
	}
	if (entityID.HasComponent<TransformComponent>())
	{
		entityID.GetComponent<TransformComponent>().Save(references, entity);
	}
	if (entityID.HasComponent<CameraComponent>())
	{
		entityID.GetComponent<CameraComponent>().Save(references, entity);
	}
	if (entityID.HasComponent<LightComponent>())
	{
		entityID.GetComponent<LightComponent>().Save(references, entity);
	}
	if (entityID.HasComponent<SkyboxComponent>())
	{
		entityID.GetComponent<SkyboxComponent>().Save(references, entity);
	}
}

void Scene::SaveToFile(const std::string& filepath)
{
	std::string _filepath = filepath;
	if (_filepath.empty())
	{
		if (!m_Filepath.empty())
			_filepath = m_Filepath;
		else
			return;
	}

	nlohmann::ordered_json scene_gsf_json;
	nlohmann::ordered_json& scene = scene_gsf_json["scene"];

	//Use filename as debugName.
	scene["debugName"] = m_CI.debugName = std::filesystem::path(_filepath).filename().generic_string();
	scene["uuid"] = m_UUID.AsUint64_t();

	nlohmann::ordered_json& references = scene["references"];
	nlohmann::ordered_json& entities = scene["entities"];
	m_Registry.each([&](entt::entity entityID)
		{
			Entity entity;
			entity.m_CI = { this };
			entity.m_Entity = entityID;

			nlohmann::ordered_json& _entity = entities[entity.GetUUID().AsString().c_str()];
			SaveEntity(references, _entity, entity);
		});

	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path fullSaveFilepath = cwd / std::filesystem::path(_filepath);
	std::filesystem::path fullSaveFileDir = cwd / std::filesystem::path(_filepath).remove_filename();

	if (_filepath.find(".gsf") == std::string::npos)
		fullSaveFilepath += ".gsf";
	
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
