#pragma once
#include "gear_core_common.h"
#include "Scene/Scene.h"

namespace gear 
{
namespace build
{
	class Project
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			Ref<graphics::Window>	window;
			std::filesystem::path	folderPath;
			bool					createDirectories;
		};

		//Methods
	public:
		Project(CreateInfo* pCreateInfo);
		~Project();

		void AddScene(Ref<scene::Scene> scene);
		void RemoveScene(Ref<scene::Scene> scene);
		inline std::vector<Ref<scene::Scene>>& GetScenes() { return m_Scenes; }
		
		//Can return nullptr.
		inline Ref<scene::Scene> GetSelectedScene()
		{
			return m_SelectedScene;
		}
		inline void SetSelectedScene(Ref<scene::Scene> scene)
		{
			bool found = false;
			for (const auto& _scene : m_Scenes)
			{
				if (_scene == scene)
				{
					found = true;
					break;
				}
			}
			if (!found)
				AddScene(scene);

			m_SelectedScene = scene;
		}

		//Members
	private:
		CreateInfo m_CI;
		std::vector<Ref<scene::Scene>> m_Scenes;
		Ref<scene::Scene> m_SelectedScene = nullptr;
	};
}
}
