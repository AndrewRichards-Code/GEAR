#pragma once
#include "UI/Panels/BasePanel.h"
#include "Scene/Entity.h"

namespace gear
{
	namespace scene
	{
		class Scene;
	}
	namespace ui
	{
		namespace panels
		{
			class GEAR_API SceneHierarchyPanel final : public Panel
			{
				//enums/structs
			public:
				struct CreateInfo
				{
					Ref<scene::Scene> scene;
				};

				//Methods
			public:
				SceneHierarchyPanel(CreateInfo* pCreateInfo);
				~SceneHierarchyPanel();

				void Update(core::Timer timer) override;
				void Draw() override;

			private:
				void DrawEntityNode(scene::Entity& entity);

			public:
				void SetScene(const Ref<scene::Scene>& scene);
				void UpdateWindowTitle();

				inline const Ref<scene::Scene>& GetScene() { return m_CI.scene; }
				inline scene::Entity& GetSelectedEntity() { return m_SelectedEntity; };
				inline CreateInfo& GetCreateInfo() { return m_CI; }

				//Members
			private:
				CreateInfo m_CI;
				scene::Entity m_SelectedEntity;
			};
		}
	}
}
