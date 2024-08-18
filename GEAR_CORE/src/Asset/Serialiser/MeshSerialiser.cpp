#include "MeshSerialiser.h"
#include "Asset/EditorAssetManager.h"

#include "Objects/Mesh.h"

using namespace gear;
using namespace asset;
using namespace serialiser;
using namespace objects;
using namespace utils;

using namespace YAML;

Ref<Asset> MeshSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	Mesh::CreateInfo meshCI;
	meshCI.device = AssetSerialiser::GetDevice();

	AssetFile assetFile(metadata);
	Node data;
	assetFile.Load(data);

	Node meshNode = data["Mesh"];

	ToType(meshCI.debugName, meshNode["debugName"]);
	meshCI.modelData = ToAsset<ModelLoader::ModelData>(meshNode["modelData"]);

	Ref<Mesh> mesh = CreateRef<Mesh>(&meshCI);
	mesh->handle = handle;

	for (const Node& materialsNode : meshNode["Materials"])
	{
		mesh->GetMaterials().push_back(ToAsset<Material>(materialsNode));
	}

	return mesh;
}

void MeshSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	AssetFile assetFile(metadata);

	const Ref<Mesh>& mesh = ref_cast<Mesh>(asset);
	const Mesh::CreateInfo& meshCI = mesh->m_CI;

	Emitter data;
	data << BeginMap;
	{
		data << Key << "Mesh" << Value << BeginSeq;
		{
			data << BeginMap;
			{
				data << Key << "debugName" << Value << meshCI.debugName;
				data << Key << "modelData" << Value << meshCI.modelData->handle;
				data << Key << "Materials" << Value << BeginSeq;
				{
					for (size_t i = 0; i < mesh->GetMaterials().size(); i++)
					{
						data << BeginMap;
						data << Key << i << Value << mesh->GetMaterial(i)->handle;
						data << EndMap;
					}
				}
				data << EndSeq;
			}
			data << EndMap;
		}
		data << EndSeq;
	}
	data << EndMap;
}