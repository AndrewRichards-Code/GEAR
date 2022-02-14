#include "gear_core_common.h"
#include "PropertiesPanel.h"
#include "Panels.h"

#include "UI/ComponentUI/NameComponentUI.h"
#include "UI/ComponentUI/TransformComponentUI.h"
#include "UI/ComponentUI/CameraComponentUI.h"
#include "UI/ComponentUI/SkyboxComponentUI.h"
#include "UI/ComponentUI/ModelComponentUI.h"

#include "Graphics/Renderer.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace mars;

PropertiesPanel::PropertiesPanel()
{
	m_Type = Type::PROPERTIES;
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Properties", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
		Ref<ViewportPanel> viewportPanel = UIContext::GetUIContext()->GetEditorPanelsByType<ViewportPanel>()[0];
		if (sceneHierarchyPanel && viewportPanel)
		{
			Entity& entity = sceneHierarchyPanel->GetSelectedEntity();
			void* device = viewportPanel->GetCreateInfo().renderer->GetDevice();
			const float& screenRatio = viewportPanel->GetCreateInfo().renderer->GetRenderSurface()->GetRatio();
			if (entity)
			{
				DrawComponentUI<NameComponent>("Name", entity, true, DrawNameComponentUI, entity);
				DrawComponentUI<TransformComponent>("Transform", entity, true, DrawTransformComponentUI, entity);
				DrawComponentUI<CameraComponent>("Camera", entity, true, DrawCameraComponentUI, entity, screenRatio);
				DrawComponentUI<SkyboxComponent>("Skybox", entity, true, DrawSkyboxComponentUI, entity);
				DrawComponentUI<ModelComponent>("Model", entity, true, DrawModelComponentUI, entity, UIContext::GetUIContext());
				
				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("AddComponents");
				
				if (ImGui::BeginPopup("AddComponents"))
				{
					if (ImGui::MenuItem("Camera"))
					{
						AddCameraComponent(entity, screenRatio, device);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Skybox"))
					{
						AddSkyboxComponent(entity, device);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Model"))
					{
						AddModelComponent(entity, device);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}
		}
	}
	ImGui::End();
}