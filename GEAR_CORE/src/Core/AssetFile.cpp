#include "gear_core_common.h"
#include "AssetFile.h"
#include "JsonFileHelper.h"
#include "FileDialog.h"
#include "Graphics/Window.h"

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

std::string AssetFile::FileDialog_Open(const Ref<graphics::Window>& window)
{
	void* _window = (void*)glfwGetWin32Window(window->GetGLFWwindow());
	return core::FileDialog_Open(_window, "GEAR Asset file (*.gaf)\0*.gaf\0");
}

std::string AssetFile::FileDialog_Save(const Ref<graphics::Window>& window)
{
	void* _window = (void*)glfwGetWin32Window(window->GetGLFWwindow());
	return core::FileDialog_Save(_window, "GEAR Asset file (*.gaf)\0*.gaf\0");
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
