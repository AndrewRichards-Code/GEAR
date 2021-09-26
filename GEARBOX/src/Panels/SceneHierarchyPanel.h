#pragma once
#include "Panel.h"
#include "ViewportPanel.h"

namespace gearbox
{
namespace panels
{
	class SceneHierarchyPanel final : public Panel
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			Ref<gear::scene::Scene> scene;
			Ref<ViewportPanel>		viewport;
		};

		//Methods
	public:
		SceneHierarchyPanel(CreateInfo* pCreateInfo);
		~SceneHierarchyPanel();

		void Draw() override;

	private:
		void DrawEntityNode(gear::scene::Entity& entity);
		
	public:
		void SetScene(const Ref<gear::scene::Scene>& scene);
		void UpdateWindowTitle();
		
		inline const Ref<gear::scene::Scene>& GetScene() { return m_CI.scene; }
		inline gear::scene::Entity& GetSelectedEntity() { return m_SelectedEntity; };
		inline CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	private:
		CreateInfo m_CI;
		gear::scene::Entity m_SelectedEntity;
	};
}
}
