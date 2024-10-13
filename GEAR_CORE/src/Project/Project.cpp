#include "gear_core_common.h"
#include "Project/Project.h"
#include "Asset/AssetManager.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EditorAssetManager.h"
#include "Asset/Serialiser/AssetSerialiser.h"
#include "Graphics/Window.h"
#include "Scene/Scene.h"
#include "yaml-cpp/yaml.h"
#include <fstream>

using namespace gear;
using namespace project;
using namespace scene;

using namespace YAML;

Project* Project::s_ActiveProject = nullptr;
Ref<asset::AssetManager> Project::s_AssetManager = nullptr;

Project::Project(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	asset::AssetRegistry::CreateInfo assetRegCI;
	assetRegCI.filepath = GetProjectAssetRegistryFilepath();
	assetRegCI.fileType = asset::AssetRegistry::FileType::TEXT;
	asset::AssetManager::CreateInfo assetManagerCI;
	assetManagerCI.pAssetRegistryCreateInfo = &assetRegCI;
	assetManagerCI.device = m_CI.window->GetDevice();
	s_AssetManager = CreateRef<asset::EditorAssetManager>(&assetManagerCI);

	if (!s_ActiveProject)
		s_ActiveProject = this;

	std::filesystem::path filepath = GetProjectFilepath();

	if (std::filesystem::exists(filepath))
	{
		Load();
	}
	else
	{
		std::filesystem::create_directory(m_CI.folderPath / "Assets");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Audio");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Fonts");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Images");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Models");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Scripts");
		std::filesystem::create_directory(m_CI.folderPath / "Scenes");

		Save();
	}
}

Project::~Project()
{
	Save();
	m_Scenes.clear();
}

void Project::Load()
{
	std::filesystem::path filepath = GetProjectFilepath();

	std::ifstream stream(filepath);
	if (!stream.is_open())
	{
		GEAR_WARN(ErrorCode::PROJECT | ErrorCode::LOAD_FAILED, "Unable to open Project file: %s.", filepath.generic_string().c_str());
		return;
	}

	Node loadData = YAML::Load(stream);
	Node scenes = loadData["Scenes"];
	{
		for (const Node& sceneFilpath : scenes)
		{
			Ref<Scene> scene = asset::serialiser::ToAsset<Scene>(sceneFilpath);
			AddScene(scene);
		}
	}
}

void Project::Save() const
{
	std::filesystem::path filepath = GetProjectFilepath();

	Emitter data;
	data << BeginMap;
	data << Key << "GEAR_PROJECT_FILE" << Value << m_CI.folderPath.stem().string();
	data << Key << "Scenes" << Value << BeginSeq;
	for (const auto& scene : m_Scenes)
	{
		data << scene->handle;
	}
	data << EndSeq;
	data << EndMap;

	std::ofstream stream(filepath);
	stream << data.c_str();
	stream.close();
}

void Project::AddScene(const Ref<Scene>& scene)
{
	m_Scenes.push_back(scene);
}


void Project::RemoveScene(const Ref<Scene>& scene)
{
	for (auto it = m_Scenes.begin(); it != m_Scenes.end();)
	{
		Ref<Scene>& _scene = *it;
		if (_scene == scene)
		{
			it = m_Scenes.erase(it);

			if (m_SelectedScene == scene)
				m_SelectedScene = (it != m_Scenes.end() ? *it : nullptr);
		}
		else
		{
			it++;
		}
	}
}

Ref<asset::AssetManager> Project::GetAssetManager()
{
	return s_AssetManager;
}

Ref<asset::EditorAssetManager> Project::GetEditorAssetManager()
{
	return ref_cast<asset::EditorAssetManager>(s_AssetManager);
}

const std::filesystem::path Project::GetProjectFolderPath() const 
{
	return m_CI.folderPath;
}

const std::filesystem::path Project::GetProjectFilepath() const
{
	const std::filesystem::path& folderPath = GetProjectFolderPath();
	return folderPath / (folderPath.stem().generic_string() + ".gear");
}

const std::filesystem::path Project::GetProjectAssetRegistryFilepath() const
{
	const std::filesystem::path& folderPath = GetProjectFolderPath();
	return folderPath / (folderPath.stem().generic_string() + ".gar");
}
