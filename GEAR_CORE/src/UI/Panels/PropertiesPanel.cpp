#include "gear_core_common.h"
#include "UI/Panels/PropertiesPanel.h"
#include "UI/Panels/ViewportPanel.h"
#include "UI/Panels/SceneHierarchyPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"

#include "Graphics/Rendering/Renderer.h"

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
			DrawCheckbox("Use Selected Entity", m_UseSelectedEntity, 200.0f);
			
			if (!m_UseSelectedEntity)
			{
				std::vector<std::string> entityNames;
				std::vector<size_t> entityIDs;
				Ref<Scene>& scene = sceneHierarchyPanel->GetCreateInfo().scene;
				entt::registry& reg = scene->GetRegistry();
				reg.each([&](entt::entity entityID)
					{
						Entity _entity({ scene.get() }, entityID);
						const std::string& name = _entity.GetComponent<NameComponent>().name;
						entityNames.push_back(name);
						entityIDs.push_back(static_cast<size_t>(entityID));
					});


				static size_t entityListIndex = 0;
				DrawDropDownMenu<size_t>("Entities", entityNames, entityListIndex);

				entity = sceneHierarchyPanel->GetSelectedEntity();
				entity.m_CI = { scene.get() };
				entity.m_Entity = static_cast<entt::entity>(entityIDs[entityListIndex]);
				
			}
			else
			{
				entity = sceneHierarchyPanel->GetSelectedEntity();
			}

			void* device = viewportPanel->GetRenderer()->GetDevice();
			const float& screenRatio = viewportPanel->GetRenderer()->GetRenderSurface()->GetRatio();
			if (entity)
			{
				DrawComponentUI<NameComponent>("Name", entity, true, DrawNameComponentUI, entity);
				DrawComponentUI<TransformComponent>("Transform", entity, true, DrawTransformComponentUI, entity);
				DrawComponentUI<CameraComponent>("Camera", entity, true, DrawCameraComponentUI, entity);
				DrawComponentUI<LightComponent>("Light", entity, true, DrawLightComponentUI, entity);
				DrawComponentUI<SkyboxComponent>("Skybox", entity, true, DrawSkyboxComponentUI, entity);
				DrawComponentUI<ModelComponent>("Model", entity, true, DrawModelComponentUI, entity);
				
				if (ImGui::Button("Add Component"))
				{
					ImGui::OpenPopup("AddComponents");
				}

				if (ImGui::BeginPopup("AddComponents"))
				{
					if (ImGui::MenuItem("Camera"))
					{
						AddCameraComponent(entity, device);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Light"))
					{
						AddLightComponent(entity, device);
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