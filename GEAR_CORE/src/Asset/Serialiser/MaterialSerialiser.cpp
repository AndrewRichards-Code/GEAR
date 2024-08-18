#include "MaterialSerialiser.h"

#include "Objects/Material.h"

using namespace gear;
using namespace asset;
using namespace serialiser;
using namespace graphics;
using namespace objects;

using namespace YAML;

Ref<Asset> MaterialSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	void* device = AssetSerialiser::GetDevice();

	Material::CreateInfo materialCI;
	materialCI.device = device;

	AssetFile assetFile(metadata);
	Node data;
	assetFile.Load(data);

	Node materialNode = data["Material"];

	Node pbrTexturesNode = materialNode["PBRTextures"];
	for (const auto& pbrTextureNode : pbrTexturesNode)
	{
		Material::TextureType type = Material::TextureType::UNKNOWN;
		ToTypeEnum(type, pbrTextureNode.first);
		materialCI.pbrTextures[type] = ToAsset<Texture>(pbrTextureNode.second);
	}

	Node pbrConstantsNode = materialNode["PBRConstants"];
	ToType(materialCI.debugName, materialNode["debugName"]);
	ToTypeVector(materialCI.pbrConstants.fresnel, pbrConstantsNode["fresnel"]);
	ToTypeVector(materialCI.pbrConstants.albedo, pbrConstantsNode["albedo"]);
	ToType(materialCI.pbrConstants.metallic, pbrConstantsNode["metallic"]);
	ToType(materialCI.pbrConstants.roughness, pbrConstantsNode["roughness"]);
	ToType(materialCI.pbrConstants.ambientOcclusion, pbrConstantsNode["ambientOcclusion"]);
	ToTypeVector(materialCI.pbrConstants.emissive, pbrConstantsNode["emissive"]);

	Ref<Material> material = CreateRef<Material>(&materialCI);
	material->handle = handle;
	return material;
}

void MaterialSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	AssetFile assetFile(metadata);

	const Ref<Material>& material = ref_cast<Material>(asset);
	const Material::CreateInfo& materialCI = material->m_CI;

	Emitter data;
	data << BeginMap;
	{
		data << Key << "Material" << Value << BeginSeq;
		{
			data << Key << "PBRTextures" << Value << BeginSeq;
			{
				for (const auto pbrTexture : materialCI.pbrTextures)
				{
					data << BeginMap;
					data << Key << ToString(pbrTexture.first) << Value << pbrTexture.second->handle;
					data << EndMap;
				}
			}
			data << EndSeq;

			data << Key << "PBRConstants" << Value << BeginSeq;
			{
				data << BeginMap;
				{
					data << Key << "debugName" << Value << materialCI.debugName;
					data << Key << "fresnel" << Value << materialCI.pbrConstants.fresnel;
					data << Key << "albedo" << Value << materialCI.pbrConstants.albedo;
					data << Key << "metallic" << Value << materialCI.pbrConstants.metallic;
					data << Key << "roughness" << Value << materialCI.pbrConstants.roughness;
					data << Key << "ambientOcclusion" << Value << materialCI.pbrConstants.ambientOcclusion;
					data << Key << "emissive" << Value << materialCI.pbrConstants.emissive;
				}
				data << EndMap;
			}
			data << EndSeq;
		}
		data << EndSeq;
	}
	data << EndMap;

	assetFile.Save(data);
}