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
		Ref<Camera> camera;

		GEAR_SCENE_COMPONENTS_DEFAULTS(CameraComponent);
		CameraComponent(Ref<Camera>& _camera) : camera(_camera) {}
		CameraComponent(Ref<Camera>&& _camera) : camera(std::move(_camera)) {}
		CameraComponent(Camera::CreateInfo* pCreateInfo) : camera(CreateRef<Camera>(pCreateInfo)) {}

		Camera::CreateInfo& GetCreateInfo() { return camera->m_CI; }

		operator Ref<Camera>&() { return camera; }
	};

	struct LightComponent
	{
		Ref<Light> light;

		GEAR_SCENE_COMPONENTS_DEFAULTS(LightComponent);
		LightComponent(Ref<Light>& _light) : light(_light) {}
		LightComponent(Ref<Light>&& _light) : light(std::move(_light)) {}
		LightComponent(Light::CreateInfo* pCreateInfo) : light(CreateRef<Light>(pCreateInfo)) {}

		Light::CreateInfo& GetCreateInfo() { return light->m_CI; }

		operator Ref<Light>&() { return light; }
	};

	struct ModelComponent
	{
		Ref<Model> model;

		GEAR_SCENE_COMPONENTS_DEFAULTS(ModelComponent);
		ModelComponent(Ref<Model>& _model) : model(_model) {}
		ModelComponent(Ref<Model>&& _model) : model(std::move(_model)) {}
		ModelComponent(Model::CreateInfo* pCreateInfo) : model(CreateRef<Model>(pCreateInfo)) {}

		Model::CreateInfo& GetCreateInfo() { return model->m_CI; }

		operator Ref<Model>&() { return model; }
	};

	struct SkyboxComponent
	{
		Ref<Skybox> skybox;

		GEAR_SCENE_COMPONENTS_DEFAULTS(SkyboxComponent);
		SkyboxComponent(Ref<Skybox>& _skybox) : skybox(_skybox) {}
		SkyboxComponent(Ref<Skybox>&& _skybox) : skybox(std::move(_skybox)) {}
		SkyboxComponent(Skybox::CreateInfo* pCreateInfo) : skybox(CreateRef<Skybox>(pCreateInfo)) {}

		Skybox::CreateInfo& GetCreateInfo() { return skybox->m_CI; }

		operator Ref<Skybox>& () { return skybox; }
	};

	struct TextComponent
	{
		Ref<Text> text;

		GEAR_SCENE_COMPONENTS_DEFAULTS(TextComponent);
		TextComponent(Ref<Text>& _model) : text(_model) {}
		TextComponent(Ref<Text>&& _model) : text(std::move(_model)) {}
		TextComponent(Text::CreateInfo* pCreateInfo) : text(CreateRef<Text>(pCreateInfo)) {}

		//Text::CreateInfo& GetCreateInfo() { return text->m_CI; }

		operator Ref<Text>& () { return text; }
	};

	struct MeshComponent
	{
		Ref<Mesh> mesh;

		GEAR_SCENE_COMPONENTS_DEFAULTS(MeshComponent);
		MeshComponent(Ref<Mesh>& _mesh) : mesh(_mesh) {}
		MeshComponent(Ref<Mesh>&& _mesh) : mesh(std::move(_mesh)) {}
		MeshComponent(Mesh::CreateInfo* pCreateInfo) : mesh(CreateRef<Mesh>(pCreateInfo)) {}

		Mesh::CreateInfo& GetCreateInfo() { return mesh->m_CI; }

		operator Ref<Mesh>&() { return mesh; }
	};

	struct MaterialComponent
	{
		Ref<Material> material;

		GEAR_SCENE_COMPONENTS_DEFAULTS(MaterialComponent);
		MaterialComponent(Ref<Material>& _material) : material(_material) {}
		MaterialComponent(Ref<Material>&& _material) : material(std::move(_material)) {}
		MaterialComponent(Material::CreateInfo* pCreateInfo) : material(CreateRef<Material>(pCreateInfo)) {}

		Material::CreateInfo& GetCreateInfo() { return material->m_CI; }

		operator Ref<Material>&() { return material; }
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
