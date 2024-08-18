#include "gear_core_common.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Scene/NativeScriptManager.h"
#include "Scene/NativeScript.h"

#include "Core/Timer.h"
#include "Core/JsonFileHelper.h"
#include "Graphics/Rendering/Renderer.h"



using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace core;
using namespace scene;
using namespace objects;

using namespace arc;

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

void Scene::OnUpdate(Ref<Renderer> renderer, Timer& timer)
{
	if (m_State == State::PLAY)
	{
		const auto& vNativeScriptComponents = m_Registry.view<NativeScriptComponent>();
		for (auto& entity : vNativeScriptComponents)
		{
			NativeScriptComponent& nativeScriptComponent = vNativeScriptComponents.get<NativeScriptComponent>(entity);
			NativeScript*& nativeScript = nativeScriptComponent.pNativeScript;
			if (!nativeScript && s_NativeScriptLibrary)
			{
				nativeScript = NativeScriptManager::LoadScript(s_NativeScriptLibrary, nativeScriptComponent.nativeScriptName);
				if (nativeScript)
				{
					nativeScript->SetEntity(*nativeScriptComponent.entity);
					nativeScript->OnCreate();
				}
			}

			if (nativeScript)
			{
				nativeScript->OnUpdate(timer);
			}
		}
	}

	const auto& vCameraComponents = m_Registry.view<TransformComponent, CameraComponent>();
	for (auto& entity : vCameraComponents)
	{
		Ref<Camera>& camera = vCameraComponents.get<CameraComponent>(entity);
		CameraComponent::Usage& usage = vCameraComponents.get<CameraComponent>(entity).usage;
		const Transform& transform = vCameraComponents.get<TransformComponent>(entity);
		camera->Update(transform);
		renderer->SubmitCamera(camera, static_cast<uint32_t>(usage));
	}

	const auto& vLightComponents = m_Registry.view<TransformComponent, LightComponent>();
	for (auto& entity : vLightComponents)
	{
		Ref<Light>& light = vLightComponents.get<LightComponent>(entity);
		const Transform& transform = vLightComponents.get<TransformComponent>(entity);
		light->m_CI.viewCamera = renderer->GetCamera();
		light->Update(transform);
		renderer->SubmitLight(light);
	}

	const auto& vModelComponents = m_Registry.view<TransformComponent, ModelComponent>();
	for (auto& entity : vModelComponents)
	{
		Ref<Model>& model = vModelComponents.get<ModelComponent>(entity);
		const Transform& transform = vModelComponents.get<TransformComponent>(entity);
		model->Update(transform);
		renderer->SubmitModel(model);
	}

	const auto& vSkyboxComponent = m_Registry.view<TransformComponent, SkyboxComponent>();
	for (auto& entity : vSkyboxComponent)
	{
		Ref<Skybox>& skybox = vSkyboxComponent.get<SkyboxComponent>(entity);
		const Transform& transform = vSkyboxComponent.get<TransformComponent>(entity);
		skybox->Update(transform);
		renderer->SubmitSkybox(skybox);
	}

	const auto& vTextComponent = m_Registry.view<TransformComponent, TextComponent>();
	for (auto& entity : vTextComponent)
	{
		Ref<Text>& text = vTextComponent.get<TextComponent>(entity);
		const Transform& transform = vTextComponent.get<TransformComponent>(entity);
		text->Update(transform);
		renderer->SubmitCamera(text->GetCamera(), static_cast<uint32_t>(CameraComponent::Usage::TEXT));
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
		NativeScriptManager::Build(m_CI.nativeScriptDir.string());
		s_NativeScriptLibrary = NativeScriptManager::Load();
	}
}

void Scene::UnloadNativeScriptLibrary()
{
	const auto& vNativeScriptComponents = m_Registry.view<NativeScriptComponent>();
	for (auto& entity : vNativeScriptComponents)
	{
		NativeScriptComponent& nativeScriptComponent = vNativeScriptComponents.get<NativeScriptComponent>(entity);
		NativeScript*& nativeScript = nativeScriptComponent.pNativeScript;
		if (nativeScript)
		{
			nativeScript->OnDestroy();
			NativeScriptManager::UnloadScript(s_NativeScriptLibrary, nativeScriptComponent.nativeScriptName, nativeScript);
		}
	}

	if (s_NativeScriptLibrary)
		NativeScriptManager::Unload(s_NativeScriptLibrary);
}
