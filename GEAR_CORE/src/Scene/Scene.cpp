#include "gear_core_common.h"
#include "Scene.h"
#include "Entity.h"
#include "NativeScriptManager.h"
#include "INativeScript.h"

#include "Core/Timer.h"
#include "Core/JsonFileHelper.h"
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
	if (m_State == State::PLAY)
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

	std::vector<Ref<objects::Light>> lights;
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
		const Ref<objects::Text>& text = vTextComponent.get<TextComponent>(entity).text;
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

using namespace nlohmann;

void Scene::LoadEntity(json& references, json& entity, Entity entityID, const Ref<graphics::Window>& window)
{
	LoadSaveParameters lsp = { this, entity, references, window };

	if (!entityID.HasComponent<UUIDComponent>())
	{
		entityID.AddComponent<UUIDComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<NameComponent>())
	{
		entityID.AddComponent<NameComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<TransformComponent>())
	{
		entityID.AddComponent<TransformComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<CameraComponent>() && JsonHasComponent<CameraComponent>(entity))
	{
		entityID.AddComponent<CameraComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<LightComponent>() && JsonHasComponent<LightComponent>(entity))
	{
		entityID.AddComponent<LightComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<ModelComponent>() && JsonHasComponent<ModelComponent>(entity))
	{
		entityID.AddComponent<ModelComponent>().Load(lsp);
	}
	if (!entityID.HasComponent<SkyboxComponent>() && JsonHasComponent<SkyboxComponent>(entity))
	{
		entityID.AddComponent<SkyboxComponent>().Load(lsp);
	}

}

void Scene::LoadFromFile(const std::string& filepath, const Ref<graphics::Window>& window)
{
	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path fullSaveFilepath = cwd / std::filesystem::path(filepath);

	s_Meshes.clear();
	s_Materials.clear();
	s_Fonts.clear();

	ClearEntities();

	json scene_gsf;
	core::LoadJsonFile(fullSaveFilepath.string(), ".gsf", "GEAR_SCENE_FILE", scene_gsf);

	json& scene = scene_gsf["scene"];
	m_CI.debugName = scene["debugName"];
	m_UUID = core::UUID(scene["uuid"]);

	json& entities = scene["entities"];
	json& references = scene["references"];

	for (auto& entity_json : entities)
	{
		Entity::CreateInfo entityCI = { this };
		Entity entity(&entityCI);

		LoadEntity(references, entity_json, entity, window);
	}

	m_Filepath = filepath;
}

void Scene::SaveEntity(json& references, json& entity, Entity entityID)
{
	LoadSaveParameters lsp = { this, entity, references, nullptr };

	if (entityID.HasComponent<UUIDComponent>())
	{
		entityID.GetComponent<UUIDComponent>().Save(lsp);
	}
	if (entityID.HasComponent<NameComponent>())
	{
		entityID.GetComponent<NameComponent>().Save(lsp);
	}
	if (entityID.HasComponent<TransformComponent>())
	{
		entityID.GetComponent<TransformComponent>().Save(lsp);
	}
	if (entityID.HasComponent<CameraComponent>())
	{
		entityID.GetComponent<CameraComponent>().Save(lsp);
	}
	if (entityID.HasComponent<LightComponent>())
	{
		entityID.GetComponent<LightComponent>().Save(lsp);
	}
	if (entityID.HasComponent<ModelComponent>())
	{
		entityID.GetComponent<ModelComponent>().Save(lsp);
	}
	if (entityID.HasComponent<SkyboxComponent>())
	{
		entityID.GetComponent<SkyboxComponent>().Save(lsp);
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

	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path fullSaveFilepath = cwd / std::filesystem::path(_filepath);

	s_Meshes.clear();
	s_Materials.clear();
	s_Fonts.clear();

	json scene_gsf;
	json& scene = scene_gsf["scene"];

	//Use filename as debugName.
	scene["debugName"] = m_CI.debugName = std::filesystem::path(_filepath).filename().generic_string();
	scene["uuid"] = m_UUID.AsUint64_t();

	json& entities = scene["entities"];
	json& references = scene["references"];
	m_Registry.each([&](entt::entity entityID)
		{
			Entity entity;
			entity.m_CI = { this };
			entity.m_Entity = entityID;

			json& _entity = entities[entity.GetUUID().AsString().c_str()];
			SaveEntity(references, _entity, entity);
		});

	core::SaveJsonFile(fullSaveFilepath.string(), ".gsf", "GEAR_SCENE_FILE", scene_gsf);
}
