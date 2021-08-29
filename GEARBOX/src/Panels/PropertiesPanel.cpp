#include "gearbox_common.h"
#include "PropertiesPanel.h"

#include "ComponentUI/NameComponentUI.h"
#include "ComponentUI/TransformComponentUI.h"
#include "ComponentUI/CameraComponentUI.h"
#include "ComponentUI/SkyboxComponentUI.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace mars;

using namespace gearbox;
using namespace panels;
using namespace componentui;

PropertiesPanel::PropertiesPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::PROPERTIES;
	m_CI = *pCreateInfo;
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::Draw()
{
	if (ImGui::Begin("Properties", &m_Open))
	{
		Entity& entity = m_CI.sceneHeirarchyPanel->GetSelectedEntity();
		void* device = m_CI.sceneHeirarchyPanel->GetCreateInfo().viewport->GetCreateInfo().renderer->GetDevice();
		const float& screenRatio = m_CI.sceneHeirarchyPanel->GetCreateInfo().viewport->GetCreateInfo().renderer->GetRenderSurface()->GetRatio();
		if (entity)
		{
			DrawNameComponentUI(entity);
			DrawTransformComponentUI(entity);

			DrawComponentUI<CameraComponent>("Camera", entity, DrawCameraComponentUI, entity, screenRatio);
			DrawComponentUI<SkyboxComponent>("Skybox", entity, DrawSkyboxComponentUI, entity);
			
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
				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}