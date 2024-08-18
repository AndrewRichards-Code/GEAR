#include "SceneSerialiser.h"

#include "Asset/AssetManager.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace asset;
using namespace serialiser;
using namespace scene;
using namespace objects;

using namespace YAML;

Ref<Asset> SceneSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	Scene::CreateInfo sceneCI;

	AssetFile assetFile(metadata);
	Node data;
	assetFile.Load(data);

	Node sceneNode = data["Scene"];
	ToType(sceneCI.debugName, sceneNode["debugName"]);

	Ref<Scene> scence = CreateRef<Scene>(&sceneCI);
	scence->handle = handle;

	Node entitiesNode = data["Entities"];
	for (const Node& entity : entitiesNode)
	{
		DeserialiseEntity(entity, scence);
	}
	
	return scence;
}

void SceneSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	AssetFile assetFile(metadata);

	const Ref<Scene>& scene = ref_cast<scene::Scene>(asset);
	const Scene::CreateInfo& sceneCI = scene->m_CI;

	Emitter data;
	data << BeginMap;
	{
		data << Key << "Scene" << Value << BeginSeq;
		{
			data << BeginMap;
			{
				data << Key << "debugName" << Value << sceneCI.debugName;
			}
			data << EndMap;
		}
		data << EndSeq; //Scene
		
		data << Key << "Entities" << Value << BeginSeq; 
		{
			scene->GetRegistry().each([&](entt::entity entityID)
			{
					Entity entity;
					entity.m_CI = { scene.get() };
					entity.m_Entity = entityID;

					SerialiseEntity(data, entity);
			});
		}
		data << EndSeq;
	}
	data << EndMap;

	assetFile.Save(data);
}

void SceneSerialiser::DeserialiseEntity(const Node& data, Ref<Scene>& scene)
{
	void* device = AssetSerialiser::GetDevice();

	Entity::CreateInfo entityCI = { scene.get() };
	Entity entity(&entityCI);
	
	uint64_t uuidRaw = 0;
	ToType(uuidRaw, data["Entity"]);
	entity.AddComponent<UUIDComponent>().uuid = uuidRaw;
	
	const Node& nameComponentNode = data["NameComponent"];
	if (nameComponentNode)
	{
		NameComponent& nameComponent = entity.AddComponent<NameComponent>();
		ToType(nameComponent.name, nameComponentNode["name"]);
	}
	
	const Node& transformComponentNode = data["TransformComponent"];
	if (transformComponentNode)
	{
		TransformComponent& transformComponent = entity.AddComponent<TransformComponent>();
		ToTypeVector(transformComponent.transform.translation, transformComponentNode["transform"]["translation"]);
		ToTypeQuaternion(transformComponent.transform.orientation, transformComponentNode["transform"]["orientation"]);
		ToTypeVector(transformComponent.transform.scale, transformComponentNode["transform"]["scale"]);
	}

	const Node& cameraComponentNode = data["CameraComponent"];
	if (cameraComponentNode)
	{
		Camera::CreateInfo cameraCI;
		cameraCI.device = device;
		ToType(cameraCI.debugName, cameraComponentNode["debugName"]);
		ToTypeEnum(cameraCI.projectionType, cameraComponentNode["projectionType"]);
		ToType(cameraCI.orthographicParams.left, cameraComponentNode["orthographicParams"]["left"]);
		ToType(cameraCI.orthographicParams.right, cameraComponentNode["orthographicParams"]["right"]);
		ToType(cameraCI.orthographicParams.bottom, cameraComponentNode["orthographicParams"]["bottom"]);
		ToType(cameraCI.orthographicParams.top, cameraComponentNode["orthographicParams"]["top"]);
		ToType(cameraCI.orthographicParams.near, cameraComponentNode["orthographicParams"]["near"]);
		ToType(cameraCI.orthographicParams.far, cameraComponentNode["orthographicParams"]["far"]);
		ToType(cameraCI.perspectiveParams.horizonalFOV, cameraComponentNode["perspectiveParams"]["horizonalFOV"]);
		ToType(cameraCI.perspectiveParams.aspectRatio, cameraComponentNode["perspectiveParams"]["aspectRatio"]);
		ToType(cameraCI.perspectiveParams.zNear, cameraComponentNode["perspectiveParams"]["zNear"]);
		ToType(cameraCI.perspectiveParams.zFar, cameraComponentNode["perspectiveParams"]["zFar"]);
		entity.AddComponent<CameraComponent>(&cameraCI);
	}

	const Node& lightComponentNode = data["LightComponent"];
	if (lightComponentNode)
	{
		Light::CreateInfo lightCI;
		lightCI.device = device;
		ToType(lightCI.debugName, lightComponentNode["debugName"]);
		ToTypeEnum(lightCI.type, lightComponentNode["type"]);
		ToTypeVector(lightCI.colour, lightComponentNode["colour"]);
		ToType(lightCI.spotInnerAngle, lightComponentNode["spotInnerAngle"]);
		ToType(lightCI.spotOuterAngle, lightComponentNode["spotOuterAngle"]);
		 entity.AddComponent<LightComponent>(&lightCI);
	}

	const Node& modelComponentNode = data["ModelComponent"];
	if (modelComponentNode)
	{
		Model::CreateInfo modelCI;
		modelCI.device = device;
		ToType(modelCI.debugName, modelComponentNode["debugName"]);
		modelCI.mesh = ToAsset<Mesh>(modelComponentNode["meshHandle"]);
		ToTypeVector(modelCI.materialTextureScaling, modelComponentNode["materialTextureScaling"]);
		ToType(modelCI.renderPipelineName, modelComponentNode["renderPipelineName"]);
		entity.AddComponent<ModelComponent>(&modelCI);
	}

	const Node& skyboxComponentNode = data["SkyboxComponent"];
	if (skyboxComponentNode)
	{
		Skybox::CreateInfo skyboxCI;
		skyboxCI.device = device;
		ToType(skyboxCI.debugName, skyboxComponentNode["debugName"]);
		skyboxCI.textureData = ToAsset<ImageAssetDataBuffer>(modelComponentNode["textureDataHandle"]);
		ToType(skyboxCI.generatedCubemapSize, modelComponentNode["generatedCubemapSize"]);
		entity.AddComponent<SkyboxComponent>(&skyboxCI);
	}
}

void SceneSerialiser::SerialiseEntity(Emitter& data, Entity& entity)
{
	data << BeginMap;
	{
		data << Key << "Entity" << Value << entity.GetUUID();

		if (entity.HasComponent<NameComponent>())
		{
			data << Key << "NameComponent";
			data << BeginMap;
			{
				const NameComponent& nameComponent = entity.GetComponent<NameComponent>();
				data << Key << "name" << Value << nameComponent.name;
			}
			data << EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			data << Key << "TransformComponent";
			data << BeginMap;
			{
				const TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
				data << BeginMap;
				data << Key << "transform";
				{
					data << Key << "translation" << Value << transformComponent.transform.translation;
					data << Key << "orientation" << Value << transformComponent.transform.orientation;
					data << Key << "scale" << Value << transformComponent.transform.scale;
				}
				data << EndMap;
			}
			data << EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			data << Key << "CameraComponent";
			data << BeginMap;
			{
				const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
				const Camera::CreateInfo& cameraCI = cameraComponent.GetCreateInfo();
				data << Key << "debugName" << Value << cameraCI.debugName;
				data << Key << "projectionType" << ToString(cameraCI.projectionType);
				data << Key << "orthographicParams";
				data << BeginMap;
				{
					data << Key << "left" << Value << cameraCI.orthographicParams.left;
					data << Key << "right" << Value << cameraCI.orthographicParams.right;
					data << Key << "bottom" << Value << cameraCI.orthographicParams.bottom;
					data << Key << "top" << Value << cameraCI.orthographicParams.top;
					data << Key << "near" << Value << cameraCI.orthographicParams.near;
					data << Key << "far" << Value << cameraCI.orthographicParams.far;
	
				}
				data << EndMap;
				data << Key << "perspectiveParams";
				data << BeginMap;
				{
					data << Key << "horizonalFOV" << Value << cameraCI.perspectiveParams.horizonalFOV;
					data << Key << "aspectRatio" << Value << cameraCI.perspectiveParams.aspectRatio;
					data << Key << "zNear" << Value << cameraCI.perspectiveParams.zNear;
					data << Key << "zFar" << Value << cameraCI.perspectiveParams.zFar;
				}
				data << EndMap;
			}
			data << EndMap;
		}

		if (entity.HasComponent<LightComponent>())
		{
			data << Key << "LightComponent";
			data << BeginMap;
			{
				const LightComponent& lightComponent = entity.GetComponent<LightComponent>();
				const Light::CreateInfo& lightCI = lightComponent.GetCreateInfo();
				data << Key << "debugName" << Value << lightCI.debugName;
				data << Key << "type" << Value << ToString(lightCI.type);
				data << Key << "colour" << Value << lightCI.colour;
				data << Key << "spotInnerAngle" << Value << lightCI.spotInnerAngle;
				data << Key << "spotOuterAngle" << Value << lightCI.spotOuterAngle;
			}
			data << EndMap;
		}

		if (entity.HasComponent<ModelComponent>())
		{
			data << Key << "ModelComponent";
			data << BeginMap;
			{
				const ModelComponent& modelComponent = entity.GetComponent<ModelComponent>();
				const Model::CreateInfo& modelCI = modelComponent.GetCreateInfo();
				data << BeginMap;
				data << Key << "debugName" << Value << modelCI.debugName;
				data << Key << "meshHandle" << Value << modelCI.mesh->handle;
				data << Key << "materialTextureScaling" << Value << modelCI.materialTextureScaling;
				data << Key << "renderPipelineName" << Value << modelCI.renderPipelineName;
			}
			data << EndMap;
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			data << Key << "SkyboxComponent";
			data << BeginMap;
			{
				const SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
				const Skybox::CreateInfo& skyboxCI = skyboxComponent.GetCreateInfo();
				data << Key << "debugName" << Value << skyboxCI.debugName;
				data << Key << "textureDataHandle" << Value << skyboxCI.textureData->handle;
				data << Key << "generatedCubemapSize" << Value << skyboxCI.generatedCubemapSize;
			}
			data << EndMap;
		}
	}
	data << EndMap;
}
