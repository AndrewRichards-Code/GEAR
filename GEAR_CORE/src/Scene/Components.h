#pragma once

#include "Core/UUID.h"
#include "Graphics/Window.h"
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Model.h"
#include "Objects/Skybox.h"
#include "Objects/Text.h"
#include "Objects/Transform.h"

namespace gear
{
namespace scene
{
	class Scene;
	struct LoadSaveParameters
	{
		scene::Scene*					scene;
		nlohmann::json&					entity;
		nlohmann::json&					references;
		const Ref<graphics::Window>&	window;
	};

	#define GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(_struct)	\
	_struct() = default;										\
	_struct(const _struct&) = default;							\
	virtual ~##_struct() = default;								\
	void Load(LoadSaveParameters&);								\
	void Save(LoadSaveParameters&);								\
	static const std::string s_StructName;
	
	#define GEAR_SCENE_COMPONENTS_DEFAULTS_DEFINITION(_struct)	\
	const std::string gear::scene::_struct::s_StructName = #_struct;


	struct GEAR_API UUIDComponent
	{
		core::UUID uuid;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(UUIDComponent);
		UUIDComponent(const uint64_t _uuid) : uuid(_uuid) {}

		operator const core::UUID&() const { return uuid; }
	};

	struct GEAR_API NameComponent
	{
		std::string name;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(NameComponent);
		NameComponent(const std::string& _name) : name(_name) {}

		operator std::string&() { return name; }
	};

	struct GEAR_API TransformComponent
	{
		objects::Transform transform;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(TransformComponent);
		TransformComponent(const objects::Transform& _transform) : transform(_transform) {}

		operator objects::Transform&() { return transform; }
		operator mars::Mat4() { return objects::TransformToMat4(transform); }

		static void Load(const nlohmann::json& parent, objects::Transform& transform);
		static void Save(nlohmann::json& parent, const objects::Transform& transform);
	};

	struct GEAR_API CameraComponent
	{
		Ref<objects::Camera> camera;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(CameraComponent);
		CameraComponent(Ref<objects::Camera>& _camera) : camera(_camera) {}
		CameraComponent(Ref<objects::Camera>&& _camera) : camera(std::move(_camera)) {}
		CameraComponent(objects::Camera::CreateInfo* pCreateInfo) : camera(CreateRef<objects::Camera>(pCreateInfo)) {}

		objects::Camera::CreateInfo& GetCreateInfo() { return camera->m_CI; }

		operator Ref<objects::Camera>&() { return camera; }
	};

	struct GEAR_API LightComponent
	{
		Ref<objects::Light> light;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(LightComponent);
		LightComponent(Ref<objects::Light>& _light) : light(_light) {}
		LightComponent(Ref<objects::Light>&& _light) : light(std::move(_light)) {}
		LightComponent(objects::Light::CreateInfo* pCreateInfo) : light(CreateRef<objects::Light>(pCreateInfo)) {}

		objects::Light::CreateInfo& GetCreateInfo() { return light->m_CI; }

		operator Ref<objects::Light>&() { return light; }
	};

	struct GEAR_API ModelComponent
	{
		Ref<objects::Model> model;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(ModelComponent);
		ModelComponent(Ref<objects::Model>& _model) : model(_model) {}
		ModelComponent(Ref<objects::Model>&& _model) : model(std::move(_model)) {}
		ModelComponent(objects::Model::CreateInfo* pCreateInfo) : model(CreateRef<objects::Model>(pCreateInfo)) {}

		objects::Model::CreateInfo& GetCreateInfo() { return model->m_CI; }

		operator Ref<objects::Model>&() { return model; }
	};

	struct GEAR_API SkyboxComponent
	{
		Ref<objects::Skybox> skybox;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(SkyboxComponent);
		SkyboxComponent(Ref<objects::Skybox>& _skybox) : skybox(_skybox) {}
		SkyboxComponent(Ref<objects::Skybox>&& _skybox) : skybox(std::move(_skybox)) {}
		SkyboxComponent(objects::Skybox::CreateInfo* pCreateInfo) : skybox(CreateRef<objects::Skybox>(pCreateInfo)) {}

		objects::Skybox::CreateInfo& GetCreateInfo() { return skybox->m_CI; }

		operator Ref<objects::Skybox>& () { return skybox; }
	};

	struct GEAR_API TextComponent
	{
		Ref<objects::Text> text;

		GEAR_SCENE_COMPONENTS_DEFAULTS_DECLARATION(TextComponent);
		TextComponent(Ref<objects::Text>& _model) : text(_model) {}
		TextComponent(Ref<objects::Text>&& _model) : text(std::move(_model)) {}
		TextComponent(objects::Text::CreateInfo* pCreateInfo) : text(CreateRef<objects::Text>(pCreateInfo)) {}

		//Text::CreateInfo& GetCreateInfo() { return text->m_CI; }

		operator Ref<objects::Text>& () { return text; }
	};

	class Entity;
	class INativeScript;
	struct GEAR_API NativeScriptComponent
	{
		INativeScript* pNativeScript = nullptr;
		std::string nativeScriptName;
		Entity* entity = nullptr;

		NativeScriptComponent(const std::string& _nativeScriptName)
			:nativeScriptName(_nativeScriptName) {}

		operator INativeScript*() { return pNativeScript; }
	};

	template<typename T>
	bool JsonHasComponent(nlohmann::json& entity)
	{
		return !(entity[T::s_StructName].empty());
	}
}
}
