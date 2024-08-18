#pragma once

#include "Asset/AssetRegistry.h"

namespace gear
{
	namespace asset
	{
		class GEAR_API AssetManager
		{
			//enum/struct
		public:
			typedef std::map<Asset::Handle, Ref<Asset>> AssetMap;

			//Method
		public:
			AssetManager(AssetRegistry::CreateInfo* pCreateInfo);
			virtual ~AssetManager();

			operator const AssetMap& () const
			{
				return m_LoadedAssets;
			}
			const AssetRegistry& GetAssetRegistry() const
			{
				return m_AssetRegistry;
			}

			template<typename T>
			Ref<T> GetAsset(Asset::Handle handle)
			{
				return ref_cast<T>(GetAsset(handle));
			}
			virtual Ref<Asset> GetAsset(Asset::Handle handle);

			virtual void UnloadAsset(Asset::Handle handle);
			virtual void RemoveAsset(Asset::Handle handle);

			const AssetMetadata& GetMetadata(Asset::Handle handle);
			const Asset::Type& GetType(Asset::Handle handle);
			const std::filesystem::path& GetFilepath(Asset::Handle handle);

			bool IsAssetHandleValid(Asset::Handle handle);
			bool IsAssetLoaded(Asset::Handle handle);

			//Member
		protected:
			AssetRegistry m_AssetRegistry;
			AssetMap m_LoadedAssets;
		};
	}
}