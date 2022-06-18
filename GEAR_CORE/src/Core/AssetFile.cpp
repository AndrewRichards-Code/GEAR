#include "gear_core_common.h"
#include "Core/AssetFile.h"
#include "Core/JsonFileHelper.h"
#include "Core/FileDialog.h"

using namespace gear;
using namespace core;

AssetFile::AssetFile(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (std::filesystem::exists(m_CI.filepath))
	{
		Load();
	}
	else
	{
		Save();
	}
}

AssetFile::AssetFile(const std::string& filepath)
{
	CreateInfo ci = { filepath };
	*this = AssetFile(&ci);
}

AssetFile::~AssetFile()
{
}

std::string AssetFile::FileDialog_Open()
{
	return core::FileDialog_Open("GEAR Asset file", "*.gaf");
}

std::string AssetFile::FileDialog_Save()
{
	return core::FileDialog_Save("GEAR Asset file", "*.gaf");
}

void AssetFile::Load()
{
	LoadJsonFile(m_CI.filepath, ".gaf", "GEAR_ASSET_FILE", m_AssetData);
}

void AssetFile::Save()
{
	SaveJsonFile(m_CI.filepath, ".gaf", "GEAR_ASSET_FILE", m_AssetData);
}

bool AssetFile::Contains(Type type)
{
	std::string typeStr = "";
	switch (type)
	{
	default:
	case Type::NONE:
		return false;
	case Type::MATERIAL:
		typeStr = "material";
		break;
	}

	if (!typeStr.empty())
	{
		return m_AssetData.find(typeStr) != m_AssetData.end();
	}

	return false;
}
