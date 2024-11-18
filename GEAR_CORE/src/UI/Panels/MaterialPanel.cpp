#include "gear_core_common.h"
#include "UI/Panels/MaterialPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/MaterialComponentUI.h"

#include "imgui/imgui.h"

using namespace gear;
using namespace ui;
using namespace panels;
using namespace componentui;

MaterialPanel::MaterialPanel()
{
	m_Type = Panel::Type::MATERIAL;
}

MaterialPanel::~MaterialPanel()
{
}

void MaterialPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Material", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		if (m_SelectedMaterial)
		{
			DrawMaterialComponentUI(m_SelectedMaterial, UIContext::GetUIContext());
			DrawRenderPipelineComponentUI(m_SelectedRenderPipeline);
		}
	}
	ImGui::End();
}
