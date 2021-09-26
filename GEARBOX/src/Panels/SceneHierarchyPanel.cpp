#include "gearbox_common.h"
#include "SceneHierarchyPanel.h"

using namespace gear;
using namespace scene;

using namespace gearbox;
using namespace panels;

SceneHierarchyPanel::SceneHierarchyPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::SCENE_HIERARCHY;
	m_CI = *pCreateInfo;
}

SceneHierarchyPanel::~SceneHierarchyPanel()
{
}

void SceneHierarchyPanel::Draw()
{
	if (ImGui::Begin("Scene Hierarchy", &m_Open))
	{
		Ref<Scene>& scene = m_CI.scene;
		entt::registry& reg = scene->GetRegistry();

		reg.each([&](entt::entity entityID)
		{
			Entity entity;
			entity.m_CI = { scene.get() };
			entity.m_Entity = entityID;

			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = Entity();
		}

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				Entity newEntity = scene->CreateEntity();
				newEntity.GetComponent<NameComponent>().name = "Empty";
			}

			ImGui::EndPopup();
		}

	}
	ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity& entity)
{
	Ref<Scene> scene = m_CI.scene;

	const std::string& name = entity.GetComponent<NameComponent>().name;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0);
	bool opened = ImGui::TreeNodeEx((void*)entity.m_Entity, flags, name.c_str());

	if (ImGui::IsItemClicked())
	{
		m_SelectedEntity = entity;
	}

	bool deleteEntity = false;
	if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::MenuItem("Delete Entity"))
			deleteEntity = true;

		ImGui::EndPopup();
	}

	if (opened)
	{
		//DrawEntityNode(childEntity);
		ImGui::TreePop();
	}

	if (deleteEntity)
	{
		if(entity == m_SelectedEntity)
			m_SelectedEntity = Entity();

		scene->DestroyEntity(entity);
	}
}

void SceneHierarchyPanel::SetScene(const Ref<gear::scene::Scene>& scene)
{
	m_CI.scene = scene;
	m_SelectedEntity.m_Entity = entt::entity(~0);
}

void SceneHierarchyPanel::UpdateWindowTitle()
{
	const Ref<gear::graphics::Window>& window = m_CI.viewport->GetCreateInfo().uiContext->GetCreateInfo().window;
	std::string newTitle = window->GetCreateInfo().title + " - " + (m_CI.scene ? m_CI.scene->m_CI.debugName : "");

	glfwSetWindowTitle(window->GetGLFWwindow(), newTitle.c_str());
}
