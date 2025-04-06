#include "AssetRegistry.h"
#include "yaml-cpp/yaml.h"
#include <fstream>

using namespace gear::asset;

using namespace YAML;

AssetRegistry::AssetRegistry(CreateInfo* pCreateInfo)
	:m_CI(*pCreateInfo)
{
	if (!std::filesystem::exists(m_CI.filepath))
		Create();
	else
		Load();
}

AssetRegistry::~AssetRegistry()
{
	Save();
}

void AssetRegistry::AddAsset(Asset::Handle handle, const AssetMetadata& assetMetadata)
{
	m_AssetRegistry[handle] = assetMetadata;
}

void AssetRegistry::RemoveAsset(Asset::Handle handle)
{
	auto it = m_AssetRegistry.find(handle);
	if (it == m_AssetRegistry.end())
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "Asset::Handle %x not found in AssetRegistry.", handle);
	}
	else
	{
		m_AssetRegistry.erase(it);
	}
}

const AssetMetadata& AssetRegistry::GetMetadata(Asset::Handle handle)
{
	return m_AssetRegistry.at(handle);
}

void AssetRegistry::Create()
{
	std::ofstream stream(m_CI.filepath);
	stream.close();
}

void AssetRegistry::Save() const
{
	if (m_CI.fileType == FileType::NONE)
	{
		return;
	}
	else if (m_CI.fileType == FileType::BINARY)
	{
		SaveBinary();
	}
	else if (m_CI.fileType == FileType::TEXT)
	{
		SaveText();
	}
	else
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "Unknown AssetRegistry::FileType.");
	}
}

void AssetRegistry::Load()
{
	if (m_CI.fileType == FileType::NONE)
	{
		return;
	}
	else if (m_CI.fileType == FileType::BINARY)
	{
		LoadBinary();
	}
	else if (m_CI.fileType == FileType::TEXT)
	{
		LoadText();
	}
	else
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "Unknown AssetRegistry::FileType.");
	}
}

void AssetRegistry::SaveBinary() const
{
}

void AssetRegistry::LoadBinary()
{
}

void AssetRegistry::SaveText() const
{
	Emitter file;
	{
		file << BeginMap; //Root
		{
			file << Key << s_KeyAssetRegistry << Value;
			file << BeginSeq; //Entries
			for (const auto& entry : m_AssetRegistry)
			{
				const Asset::Handle& handle = entry.first;
				const AssetMetadata& metadata = entry.second;

				file << BeginMap; //Metadata
				{
					file << Key << s_KeyHandle << Value << handle;
					file << Key << s_KeyType << Value << Asset::ToString(metadata.type);
					file << Key << s_KeyFilepath << Value << metadata.filepath.generic_string();
				}
				file << EndMap; //Metadata
			}
			file << EndSeq; //Entries
		}
		file << EndMap; //Root
	}

	std::ofstream stream(m_CI.filepath);
	stream << file.c_str();
	stream.close();
}

void AssetRegistry::LoadText()
{
	std::ifstream stream(m_CI.filepath);
	if (!stream.is_open())
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::LOAD_FAILED, "Unable to open AssetRegistry file: %s.", m_CI.filepath.generic_string().c_str());
		return;
	}

	Node data = YAML::Load(stream);
	Node rootNode = data[s_KeyAssetRegistry]; //Root
	if (rootNode) 
	{
		for (const Node& entryNode : rootNode) //Entries
		{
			//Metadata
			const Asset::Handle& handle = entryNode[s_KeyHandle].as<uint64_t>();
			AssetMetadata& metadata = m_AssetRegistry[handle];
			metadata.type = Asset::FromString(entryNode[s_KeyType].as<std::string>());
			metadata.filepath = entryNode[s_KeyFilepath].as<std::string>();
		}
	}
}
