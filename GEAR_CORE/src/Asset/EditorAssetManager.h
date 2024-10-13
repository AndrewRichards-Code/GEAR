#pragma once

#include "Asset/AssetManager.h"
#include "Asset/AssetDataBuffer.h"

namespace gear
{
	namespace asset
	{
		class GEAR_API EditorAssetManager final : public AssetManager
		{
			//Method
		public:
			EditorAssetManager(CreateInfo* pCreateInfo);
			~EditorAssetManager();

			template<typename T>
			Ref<T> Import(Asset::Type type, const std::filesystem::path& filepath = "")
			{
				return ref_cast<T>(Import(type, filepath));
			}
			Ref<Asset> Import(Asset::Type type, const std::filesystem::path& filepath = "");

			template<typename T>
			void Export(Ref<T> asset, const std::filesystem::path& filepath = "")
			{
				return Export(ref_cast<Asset>(asset), filepath);
			}
			void Export(Ref<Asset> asset, const std::filesystem::path& filepath = "");
		};
	}
}