#pragma once
#include "Core/UUID.h"

namespace gear
{
	namespace asset
	{
		class GEAR_API Asset
		{
			//enum/struct
		public:
			typedef core::UUID Handle;

			enum class Type : uint32_t
			{
				NONE,
				EXTERNAL_FILE,
				TEXTURE,
				FONT,
				MATERIAL,
				MESH,
				AUDIO,
				SCENE,
			};

			//Methods
		public:
			Asset() = default;
			virtual ~Asset() = default;

			static std::string ToString(Type type);
			static Type FromString(const std::string& assetType);

			//Members
		public:
			Handle handle;
		};

		class GEAR_API AssetWithExternalAssets : public Asset
		{
			//Methods
		public:
			AssetWithExternalAssets() = default;
			virtual ~AssetWithExternalAssets() = default;

			//Members
		public:
			std::vector<Handle> externalAssets;
		};
	}
}