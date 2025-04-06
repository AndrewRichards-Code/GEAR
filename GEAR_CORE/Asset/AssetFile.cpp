#include "gear_core_common.h"
#include "Asset/AssetFile.h"
#include "yaml-cpp/yaml.h"
#include <fstream>

using namespace gear;
using namespace asset;

AssetFile::AssetFile(const AssetMetadata& metadata)
{
	m_Metadata = metadata;
}

AssetFile::~AssetFile()
{
}

void AssetFile::Load(YAML::Node& loadData)
{
	std::ifstream stream(m_Metadata.filepath);
	if (!stream.is_open())
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::LOAD_FAILED, "Unable to open AssetFile: %s.", m_Metadata.filepath.generic_string().c_str());
		return;
	}

	loadData = YAML::Load(stream);

	Asset::Type type = Asset::FromString(loadData["GEAR_ASSET_FILE"].as<std::string>());
	if (type != m_Metadata.type)
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_STATE, "AssetFile: %s Type in file: %s doesn't match: %s.", 
			m_Metadata.filepath.generic_string().c_str(),
			Asset::ToString(type).c_str(),
			Asset::ToString(m_Metadata.type).c_str()
		);
	}
}

void AssetFile::Save(const YAML::Emitter& saveData)
{
	YAML::Emitter assetHeader;
	assetHeader << YAML::BeginMap;
	assetHeader << YAML::Key << "GEAR_ASSET_FILE" << YAML::Value << Asset::ToString(m_Metadata.type);
	assetHeader << YAML::EndMap;

	std::ofstream stream(m_Metadata.filepath);
	stream << assetHeader.c_str();
	stream << saveData.c_str();
	stream.close();
}
