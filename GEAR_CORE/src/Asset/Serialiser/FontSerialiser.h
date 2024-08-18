#pragma once
#include "Asset/AssetMetadata.h"
#include "Asset/Serialiser/AssetSerialiser.h"

namespace gear
{
	namespace asset
	{
		namespace serialiser
		{
			class GEAR_API FontSerialiser
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);
			};
		}
	}
}