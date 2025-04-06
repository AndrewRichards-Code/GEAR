#pragma once
#include "gear_core_common.h"
#include "Animation/Animation.h"
#include "Objects/Material.h"
#include "Objects/Mesh.h"

#include "Asset/AssetMetadata.h"
#include "Asset/Serialiser/AssetSerialiser.h"

struct aiScene;
struct aiNode;
struct aiMaterial;

namespace gear
{
	namespace asset
	{
		namespace serialiser
		{
			class GEAR_ASSET_MANAGER_API ModelLoader
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);

			public:
				static objects::ModelData LoadModelData(const std::filesystem::path& filepath);

				inline static void SetDevice(void* device) { m_Device = device; }

			private:
				static void BuildNodeGraph(const aiScene* scene, aiNode* node, objects::MeshNode& thisNode, objects::ModelData& modelData);

				static std::vector<objects::MeshData> ProcessMeshes(aiNode* node, const aiScene* scene);
				static std::vector<animation::Animation> ProcessAnimations(const aiScene* scene);

				static void FillOutMaterialCreateInfo(aiMaterial* aiMaterial, objects::Material::CreateInfo& materialCreateInfo);

			private:
				static void* m_Device;
			};
		}
	}
}