#include "gear_core_common.h"
#include "Build/Project.h"
#include "Core/JsonFileHelper.h"
#include "Scene/Scene.h"

using namespace gear;
using namespace build;
using namespace scene;

Project::Project(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	if (m_CI.createDirectories)
	{
		std::filesystem::create_directory(m_CI.folderPath / "Assets");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Audio");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Fonts");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Images");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Models");
		std::filesystem::create_directory(m_CI.folderPath / "Assets/Scripts");
		std::filesystem::create_directory(m_CI.folderPath / "Scenes");

		nlohmann::json data;
		data["name"] = m_CI.folderPath.stem().string();
		std::string filepath = m_CI.folderPath.string() + "/" + m_CI.folderPath.stem().string() + ".gear";
		core::SaveJsonFile(filepath, ".gear", "GEAR_PROJECT_FILE", data);
	}
	else
	{
		nlohmann::json data;
		std::string filepath = m_CI.folderPath.string() + "/" + m_CI.folderPath.stem().string() + ".gear";
		core::LoadJsonFile(filepath, ".gear", "GEAR_PROJECT_FILE", data);

		for (auto& sceneFilpath : data["scenes"])
		{
			Scene::CreateInfo sceneCI = { "DefaultScene", "res/scripts/" };
			Ref<Scene> scene = CreateRef<Scene>(&sceneCI);
			scene->LoadFromFile(sceneFilpath, m_CI.window);
			AddScene(scene);
		}
	}
}

Project::~Project()
{
	nlohmann::json data;
	std::string filepath = m_CI.folderPath.string() + "/" + m_CI.folderPath.stem().string() + ".gear";
	core::LoadJsonFile(filepath, ".gear", "GEAR_PROJECT_FILE", data);

	data["scenes"].clear();
	for (auto& scene : m_Scenes)
		data["scenes"].push_back(scene->GetFilepath());
	
	core::SaveJsonFile(filepath, ".gear", "GEAR_PROJECT_FILE", data);
	
	m_Scenes.clear();
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
