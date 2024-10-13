#include "AssetManager.h"
#include "Asset/Serialiser/AssetSerialiser.h"

using namespace gear::asset;
using namespace serialiser;

AssetManager::AssetManager(CreateInfo* pCreateInfo)
	: m_AssetRegistry(pCreateInfo->pAssetRegistryCreateInfo)
{
	AssetSerialiser::SetDevice(pCreateInfo->device);
}

AssetManager::~AssetManager()
{
}

Ref<Asset> AssetManager::GetAsset(Asset::Handle handle)
{
	if (!IsAssetHandleValid(handle))
		return nullptr;

	if (IsAssetLoaded(handle))
	{
		return m_LoadedAssets.at(handle);
	}
	else
	{
		const AssetMetadata& metadata = GetMetadata(handle);
		Ref<Asset> asset = AssetSerialiser::Deserialise(handle, metadata);
		if (!asset)
		{
			GEAR_ERROR(ErrorCode::ASSET | ErrorCode::LOAD_FAILED, "Faild to load Asset with AssetHandle: %ui", handle.AsUint64_t());
		}
		asset->handle = handle;
		m_LoadedAssets[handle] = asset;

		return asset;
	}
}

void AssetManager::UnloadAsset(Asset::Handle handle)
{
	if (IsAssetHandleValid(handle) && IsAssetLoaded(handle))
	{
		m_LoadedAssets.erase(handle);
	}
}

void AssetManager::RemoveAsset(Asset::Handle handle)
{
	if (IsAssetHandleValid(handle))
	{
		UnloadAsset(handle);
		m_AssetRegistry.RemoveAsset(handle);
	}
}

const AssetMetadata& AssetManager::GetMetadata(Asset::Handle handle)
{
	static AssetMetadata NullMetaData;

	if (!IsAssetHandleValid(handle))
		return NullMetaData;

	return m_AssetRegistry.GetMetadata(handle);
}

const Asset::Type& AssetManager::GetType(Asset::Handle handle)
{
	return GetMetadata(handle).type;
}

const std::filesystem::path& AssetManager::GetFilepath(Asset::Handle handle)
{
	return GetMetadata(handle).filepath;
}

bool AssetManager::IsAssetHandleValid(Asset::Handle handle)
{
	const AssetRegistry::AssetMetadataMap& assetRegistryMap = m_AssetRegistry;
	return assetRegistryMap.find(handle) != assetRegistryMap.end();
}

bool AssetManager::IsAssetLoaded(Asset::Handle handle)
{
	return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
}