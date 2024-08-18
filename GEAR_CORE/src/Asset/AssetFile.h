#pragma once

#include "gear_core_common.h"
#include "Asset/AssetMetadata.h"

namespace YAML
{
	class Node;
	class Emitter;
}

namespace gear
{
	namespace asset
	{
		class GEAR_API AssetFile
		{
			//Methods
		public:
			AssetFile(const AssetMetadata& metadata);
			~AssetFile();

			void Load(YAML::Node& loadData);
			void Save(const YAML::Emitter& saveData);

			//Members
		public:
			AssetMetadata m_Metadata;
		};
	}
}
