#pragma once
#include "Asset/AssetMetadata.h"
#include "Asset/Serialiser/AssetSerialiser.h"

namespace gear
{
	namespace asset
	{
		namespace serialiser
		{
			class GEAR_ASSET_MANAGER_API ImageSerialiser
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);
			
			private:
				static void LoadICOData(const AssetMetadata& metadata, void*& stbiBuffer, uint32_t& width, uint32_t& height, uint32_t& channels, uint32_t components);
			};
		}
	}
}