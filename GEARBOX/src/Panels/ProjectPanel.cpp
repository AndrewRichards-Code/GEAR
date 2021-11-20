#include "gearbox_common.h"
#include "ProjectPanel.h"
#include "UIContext.h"
#include "ComponentUI/ComponentUI.h"

#include "Panels.h"

using namespace gearbox;
using namespace panels;
using namespace componentui;

using namespace gear;
using namespace scene;

ProjectPanel::ProjectPanel()
{
	m_Type = Type::PROJECT;
}

ProjectPanel::~ProjectPanel()
{
}

void ProjectPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Project", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		Ref<build::Project> project = UIContext::GetUIContext()->GetProject();
		if (project)
		{
			if (DrawTreeNode("Scenes"))
			{
				for (auto& scene : project->GetScenes())
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
					flags |= ((project->GetSelectedScene() == scene) ? ImGuiTreeNodeFlags_Selected : 0);
					bool opened = ImGui::TreeNodeEx((void*)scene.get(), flags, scene->m_CI.debugName.c_str());
					if (ImGui::IsItemClicked())
					{
						project->SetSelectedScene(scene);
					}
					bool deleteScene = false;
					if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
					{
						if (ImGui::MenuItem("Use Scene"))
						{
							Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
							if (sceneHierarchyPanel)
								sceneHierarchyPanel->SetScene(scene);
						}
						if (ImGui::MenuItem("Delete Scene"))
							deleteScene = true;

						ImGui::EndPopup();
					}
					if (opened)
					{
						ImGui::TreePop();
					}
					if (deleteScene)
					{
						project->RemoveScene(scene);
					}
				}
				EndDrawTreeNode();
			}
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".gsf");
				if (payload)
				{
					std::string filepath = (char*)payload->Data;
					if (std::filesystem::exists(filepath))
					{
						Scene::CreateInfo sceneCI = { "DefaultScene", "res/scripts/" };
						Ref<Scene> scene = CreateRef<Scene>(&sceneCI);
						scene->LoadFromFile(filepath, UIContext::GetUIContext()->GetWindow());
						project->AddScene(scene);
					}
				}
			}

			if (ImGui::Button("Add Scene"))
			{
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove Scene"))
			{
				project->RemoveScene(project->GetSelectedScene());
			}
			ImGui::Separator();
		}
	}
	ImGui::End();
}
