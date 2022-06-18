#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		class Window;
	}
	namespace scene
	{
		class Scene;
	}
	namespace build
	{
		class GEAR_API Project
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

			void AddScene(const Ref<scene::Scene>& scene);
			void RemoveScene(const Ref<scene::Scene>& scene);
			inline std::vector<Ref<scene::Scene>>& GetScenes() { return m_Scenes; }

			//Can return nullptr.
			inline Ref<scene::Scene> GetSelectedScene()
			{
				return m_SelectedScene;
			}
			inline void SetSelectedScene(const Ref<scene::Scene>& scene)
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
