#include "gearbox_common.h"
#include "PropertiesPanel.h"

#include "ComponentUI/NameComponentUI.h"
#include "ComponentUI/TransformComponentUI.h"
#include "ComponentUI/CameraComponentUI.h"

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
	if (ImGui::Begin("Properties"))
	{
		Entity& entity = m_CI.sceneHeirarchyPanel->GetSelectedEntity();
		void* device = m_CI.sceneHeirarchyPanel->GetCreateInfo().viewport->GetCreateInfo().renderer->GetDevice();
		const float& screenRatio = m_CI.sceneHeirarchyPanel->GetCreateInfo().viewport->GetCreateInfo().renderer->GetRenderSurface()->GetRatio();
		if (entity)
		{
			DrawNameComponentUI(entity);
			DrawTransformComponentUI(entity);
			//DrawCameraComponentUI(entity, screenRatio);

			DrawComponentUI<CameraComponent>("Camera", entity, DrawCameraComponentUI, entity, screenRatio);
			
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponents");
			
			if (ImGui::BeginPopup("AddComponents"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					AddCameraComponent(entity, screenRatio, device);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}