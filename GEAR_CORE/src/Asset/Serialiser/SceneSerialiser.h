#pragma once
#include "Asset/AssetMetadata.h"
#include "Asset/Serialiser/AssetSerialiser.h"

namespace gear
{
	namespace scene
	{
		class Scene;
		class Entity;
	}

	namespace asset
	{
		namespace serialiser
		{
			class GEAR_API SceneSerialiser
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);

			private:
				static void DeserialiseEntity(const YAML::Node& data, Ref<scene::Scene>& scene);
				static void SerialiseEntity(YAML::Emitter& data, scene::Entity& entity);
			};
		}
	}
}