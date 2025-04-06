#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace asset::manager
	{
		class AssetManager;
		class EditorAssetManager;
	}
	namespace graphics
	{
		class Window;
	}
	namespace scene
	{
		class Scene;
	}
	namespace project
	{
		class GEAR_PROJECT_API Project
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				void*					device;
				std::filesystem::path	folderPath;
			};

			//Methods
		public:
			Project(CreateInfo* pCreateInfo);
			~Project();

			void Load();
			void Save() const;

			void AddScene(const Ref<scene::Scene>& scene);
			void RemoveScene(const Ref<scene::Scene>& scene);
			inline std::vector<Ref<scene::Scene>>& GetScenes() { return m_Scenes; }
			
			inline const CreateInfo& GetCreateInfo() { return m_CI; }
			inline static Project* GetActiveProject() { return s_ActiveProject; }
			static Ref<asset::manager::AssetManager> GetAssetManager();
			static Ref<asset::manager::EditorAssetManager> GetEditorAssetManager();

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

			const std::filesystem::path GetProjectFolderPath() const;
			const std::filesystem::path GetProjectFilepath() const;
			const std::filesystem::path GetProjectAssetRegistryFilepath() const;

			//Members
		private:
			CreateInfo m_CI;
			std::vector<Ref<scene::Scene>> m_Scenes;
			Ref<scene::Scene> m_SelectedScene = nullptr;
			static Project* s_ActiveProject;
			static Ref<asset::manager::AssetManager> s_AssetManager;
		};
	}
}
