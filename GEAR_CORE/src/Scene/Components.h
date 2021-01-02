#pragma once

#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Model.h"
#include "Objects/Skybox.h"
#include "Objects/Text.h"
#include "Objects/Transform.h"

#define GEAR_SCENE_COMPONENTS_DEFAULTS(_struct) _struct() = default; _struct(const _struct&) = default; virtual ~##_struct() = default

namespace gear
{
namespace scene
{
	using namespace objects;

	struct NameComponent
	{
		std::string name;

		GEAR_SCENE_COMPONENTS_DEFAULTS(NameComponent);
		NameComponent(const std::string& _name) : name(_name) {}

		operator std::string&() { return name; }
	};

	struct TransformComponent
	{
		Transform transform;

		GEAR_SCENE_COMPONENTS_DEFAULTS(TransformComponent);
		TransformComponent(const Transform& _transform) : transform(_transform) {}

		operator Transform&() { return transform; }
		operator mars::Mat4() { return objects::TransformToMat4(transform); }
	};

	struct CameraComponent
	{
		gear::Ref<Camera> camera;

		GEAR_SCENE_COMPONENTS_DEFAULTS(CameraComponent);
		CameraComponent(gear::Ref<Camera>& _camera) : camera(_camera) {}
		CameraComponent(gear::Ref<Camera>&& _camera) : camera(std::move(_camera)) {}
		CameraComponent(Camera::CreateInfo* pCreateInfo) : camera(gear::CreateRef<Camera>(pCreateInfo)) {}

		Camera::CreateInfo& GetCreateInfo() { return camera->m_CI; }

		operator gear::Ref<Camera>&() { return camera; }
	};

	struct LightComponent
	{
		gear::Ref<Light> light;

		GEAR_SCENE_COMPONENTS_DEFAULTS(LightComponent);
		LightComponent(gear::Ref<Light>& _light) : light(_light) {}
		LightComponent(gear::Ref<Light>&& _light) : light(std::move(_light)) {}
		LightComponent(Light::CreateInfo* pCreateInfo) : light(gear::CreateRef<Light>(pCreateInfo)) {}

		Light::CreateInfo& GetCreateInfo() { return light->m_CI; }

		operator gear::Ref<Light>&() { return light; }
	};

	struct ModelComponent
	{
		gear::Ref<Model> model;

		GEAR_SCENE_COMPONENTS_DEFAULTS(ModelComponent);
		ModelComponent(gear::Ref<Model>& _model) : model(_model) {}
		ModelComponent(gear::Ref<Model>&& _model) : model(std::move(_model)) {}
		ModelComponent(Model::CreateInfo* pCreateInfo) : model(gear::CreateRef<Model>(pCreateInfo)) {}

		Model::CreateInfo& GetCreateInfo() { return model->m_CI; }

		operator gear::Ref<Model>&() { return model; }
	};

	struct SkyboxComponent
	{
		gear::Ref<Skybox> skybox;

		GEAR_SCENE_COMPONENTS_DEFAULTS(SkyboxComponent);
		SkyboxComponent(gear::Ref<Skybox>& _skybox) : skybox(_skybox) {}
		SkyboxComponent(gear::Ref<Skybox>&& _skybox) : skybox(std::move(_skybox)) {}
		SkyboxComponent(Skybox::CreateInfo* pCreateInfo) : skybox(gear::CreateRef<Skybox>(pCreateInfo)) {}

		Skybox::CreateInfo& GetCreateInfo() { return skybox->m_CI; }

		operator gear::Ref<Skybox>& () { return skybox; }
	};

	struct TextComponent
	{
		gear::Ref<Text> text;

		GEAR_SCENE_COMPONENTS_DEFAULTS(TextComponent);
		TextComponent(gear::Ref<Text>& _model) : text(_model) {}
		TextComponent(gear::Ref<Text>&& _model) : text(std::move(_model)) {}
		TextComponent(Text::CreateInfo* pCreateInfo) : text(gear::CreateRef<Text>(pCreateInfo)) {}

		//Text::CreateInfo& GetCreateInfo() { return text->m_CI; }

		operator gear::Ref<Text>& () { return text; }
	};

	struct MeshComponent
	{
		gear::Ref<Mesh> mesh;

		GEAR_SCENE_COMPONENTS_DEFAULTS(MeshComponent);
		MeshComponent(gear::Ref<Mesh>& _mesh) : mesh(_mesh) {}
		MeshComponent(gear::Ref<Mesh>&& _mesh) : mesh(std::move(_mesh)) {}
		MeshComponent(Mesh::CreateInfo* pCreateInfo) : mesh(gear::CreateRef<Mesh>(pCreateInfo)) {}

		Mesh::CreateInfo& GetCreateInfo() { return mesh->m_CI; }

		operator gear::Ref<Mesh>&() { return mesh; }
	};

	struct MaterialComponent
	{
		gear::Ref<Material> material;

		GEAR_SCENE_COMPONENTS_DEFAULTS(MaterialComponent);
		MaterialComponent(gear::Ref<Material>& _material) : material(_material) {}
		MaterialComponent(gear::Ref<Material>&& _material) : material(std::move(_material)) {}
		MaterialComponent(Material::CreateInfo* pCreateInfo) : material(gear::CreateRef<Material>(pCreateInfo)) {}

		Material::CreateInfo& GetCreateInfo() { return material->m_CI; }

		operator gear::Ref<Material>&() { return material; }
	};

	class Entity;
	class INativeScript;
	struct NativeScriptComponent
	{
		INativeScript* pNativeScript = nullptr;
		std::string nativeScriptName;
		Entity* entity = nullptr;

		NativeScriptComponent(const std::string& _nativeScriptName)
			:nativeScriptName(_nativeScriptName) {}

		operator INativeScript*() { return pNativeScript; }
	};
}
}
