#pragma once

#include "Asset/AssetMetadata.h"

namespace gear
{
	namespace asset
	{
		class GEAR_API AssetRegistry
		{
			//enum/struct
		public:
			typedef std::map<Asset::Handle, AssetMetadata> AssetMetadataMap;

			enum class FileType : uint8_t
			{
				NONE,
				BINARY,
				TEXT
			};
			struct CreateInfo
			{
				std::filesystem::path filepath;
				FileType fileType;
			};

			//Methods
		public:
			AssetRegistry(CreateInfo* pCreateInfo);
			~AssetRegistry();

			operator const AssetMetadataMap&() const
			{
				return m_AssetRegistry;
			}
			const std::filesystem::path& GetAssetRegistryFilepath() const
			{
				return m_CI.filepath;
			}

			void AddAsset(Asset::Handle handle, const AssetMetadata& assetMetadata);
			void RemoveAsset(Asset::Handle handle);

			const AssetMetadata& GetMetadata(Asset::Handle handle);

			void Create();
			void Save() const;
			void Load();

		private:
			void SaveBinary() const;
			void LoadBinary();

			void SaveText() const;
			void LoadText();

			//Members
		private:
			CreateInfo m_CI;
			AssetMetadataMap m_AssetRegistry;

			static constexpr const char* s_KeyAssetRegistry = "AssetRegistry";
			static constexpr const char* s_KeyHandle = "Handle";
			static constexpr const char* s_KeyType = "Type";
			static constexpr const char* s_KeyFilepath = "Filepath";
			static constexpr const char* s_KeyLastWriteTime = "LastWriteTime";
		};
	}
}