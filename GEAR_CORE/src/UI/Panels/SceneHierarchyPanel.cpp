#include "gear_core_common.h"
#include "SceneHierarchyPanel.h"
#include "Panels.h"

#include "UI/ComponentUI/ComponentUI.h"

#include "Scene/Entity.h"

using namespace gear;
using namespace core;
using namespace graphics;
using namespace scene;

using namespace ui;
using namespace panels;
using namespace componentui;

SceneHierarchyPanel::SceneHierarchyPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::SCENE_HIERARCHY;
	m_CI = *pCreateInfo;
}

SceneHierarchyPanel::~SceneHierarchyPanel()
{
}

void SceneHierarchyPanel::Update(Timer timer)
{
	Ref<ViewportPanel> viewport = UIContext::GetUIContext()->GetEditorPanelsByType<ViewportPanel>()[0];
	if (viewport)
		m_CI.scene->OnUpdate(viewport->GetCreateInfo().renderer, timer);
}

void SceneHierarchyPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Scene Hierarchy", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		Ref<Scene>& scene = m_CI.scene;
		Scene::State state = scene->GetState();
		entt::registry& reg = scene->GetRegistry();

		DrawStaticText("Controls", "");
		ImGui::NewLine();
		float iconHeight = 32.0f;
		float width = ImGui::GetContentRegionAvailWidth();
		ImGui::SameLine(width / 2.0f - iconHeight / 2.0f);
		std::string buttonStr = state == Scene::State::EDIT ? ">" : "X";
		if (ImGui::Button(buttonStr.c_str(), { iconHeight, iconHeight }))
		{
			if (state == Scene::State::EDIT)
				scene->Play();
			else if (state == Scene::State::PLAY)
				scene->Stop();
		}
		ImGui::Separator();

		DrawInputText("Name", scene->m_CI.debugName);
		ImGui::NewLine();

		ImGui::Text("Entities");
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

void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
{
	m_CI.scene = scene;
	m_SelectedEntity.m_Entity = entt::entity(~0);
}

void SceneHierarchyPanel::UpdateWindowTitle()
{
	const Ref<Window>& window = UIContext::GetUIContext()->GetWindow();
	std::string newTitle = window->GetCreateInfo().title + " - " + (m_CI.scene ? m_CI.scene->m_CI.debugName : "");

	glfwSetWindowTitle(window->GetGLFWwindow(), newTitle.c_str());
}
