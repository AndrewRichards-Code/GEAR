#include "gear_core_common.h"
#include "UI/Panels/ProjectPanel.h"
#include "UI/Panels/SceneHierarchyPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "Project/Project.h"
#include "Asset/EditorAssetManager.h"
#include "Asset/AssetRegistry.h"

using namespace gear;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace project;
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
	Ref<Project> project = UIContext::GetUIContext()->GetProject();
	
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Project", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
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
					std::filesystem::path filepath = (char*)payload->Data;
					if (std::filesystem::exists(filepath))
					{
						Ref<asset::EditorAssetManager> editorAssetManager = UIContext::GetUIContext()->GetEditorAssetManager();
						Ref<Scene> scene = editorAssetManager->Import<Scene>(asset::Asset::Type::SCENE, filepath);
						project->AddScene(scene);
					}
				}
			}
			if (ImGui::Button("Add Scene"))
			{
				Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
				if (sceneHierarchyPanel)
					project->AddScene(sceneHierarchyPanel->GetScene());
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove Scene"))
			{
				project->RemoveScene(project->GetSelectedScene());
			}
			ImGui::Separator();

			if (DrawTreeNode("Asset Registry"))
			{
				Ref<asset::AssetManager> assetManager = project->GetAssetManager();
				const asset::AssetRegistry::AssetMetadataMap& metadataMap = assetManager->GetAssetRegistry();
				for (const auto& metadata : metadataMap)
				{
					asset::Asset::Handle handle = metadata.first;

					ui::componentui::DrawStaticNumber("Handle", handle);
					ui::componentui::DrawStaticText("Type", asset::Asset::ToString(assetManager->GetType(handle)));
					ui::componentui::DrawStaticText("Filepath", assetManager->GetFilepath(handle).generic_string());
					if(ImGui::Button("View"))
					{
						ContentEditorPanel::CreateInfo contentEditorCI;
						contentEditorCI.currentFilepathFull = assetManager->GetFilepath(handle).generic_string();
						contentEditorCI.filepathExt = assetManager->GetFilepath(handle).extension().generic_string();
						contentEditorCI.handle = handle;
						UIContext::GetUIContext()->GetEditorPanels().emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
					}
					ImGui::SameLine();
					if (ImGui::Button("Remove"))
					{
						assetManager->RemoveAsset(handle);
						break;
					}
				}
				EndDrawTreeNode();
			}
			ImGui::Separator();
		}
		
	}
	ImGui::End();
}
