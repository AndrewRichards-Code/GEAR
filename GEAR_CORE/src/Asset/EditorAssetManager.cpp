#include "EditorAssetManager.h"
#include "Asset/Serialiser/AssetSerialiser.h"

using namespace gear::asset;
using namespace serialiser;

EditorAssetManager::EditorAssetManager(AssetRegistry::CreateInfo* pCreateInfo)
	: AssetManager(pCreateInfo)
{
}

EditorAssetManager::~EditorAssetManager()
{
}

Ref<Asset> EditorAssetManager::Import(Asset::Type type, const std::filesystem::path& filepath)
{
	Asset::Handle handle;
	AssetMetadata metadata;
	metadata.type = type;
	metadata.filepath = filepath;

	//Find in Asset Registry
	const AssetRegistry::AssetMetadataMap& assetRegistryMap = m_AssetRegistry;
	for (auto it = assetRegistryMap.begin(); it != assetRegistryMap.end(); it++)
	{
		if (it->second.filepath.generic_string() == filepath.generic_string())
			return GetAsset(it->first);
	}

	//Load
	Ref<Asset> asset = AssetSerialiser::Deserialise(handle, metadata);
	if (asset)
	{
		asset->handle = handle;
		m_LoadedAssets[handle] = asset;
		m_AssetRegistry.AddAsset(handle, metadata);
		m_AssetRegistry.Save();
	}

	return asset;
}

void EditorAssetManager::Export(Ref<Asset> asset, const std::filesystem::path& filepath)
{
	AssetMetadata metadata = GetMetadata(asset->handle);
	metadata.filepath = !filepath.empty() ? filepath : metadata.filepath;

	AssetSerialiser::Serialise(asset, metadata);
}
