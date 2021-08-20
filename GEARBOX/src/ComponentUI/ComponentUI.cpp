#include "gearbox_common.h"
#include "NameComponentUI.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

using namespace mars;

void gearbox::componentui::DrawInputText(const std::string& label, std::string& name, float width)
{
	BeginColumnLabel(label, width);
	ImGui::InputText("##label", &name);
	EndColumnLabel();
}

void gearbox::componentui::DrawCheckbox(const std::string& label, bool& value, float width)
{
	BeginColumnLabel(label, width);
	ImGui::Checkbox("##label", &value);
	EndColumnLabel();
}

void gearbox::componentui::DrawFloat(const std::string& label, float& value, float minValue, float maxValue, float width)
{
	BeginColumnLabel(label, width);
	ImGui::DragFloat("##label", &value, DefaultSpeed, minValue, maxValue);
	EndColumnLabel();
}

void gearbox::componentui::DrawDouble(const std::string& label, double& value, double minValue, double maxValue, float width)
{
	BeginColumnLabel(label, width);
	ImGui::DragScalar("##label", ImGuiDataType_Double, (void*)&value, DefaultSpeed, (const void*)&minValue, (const void*)&maxValue, "%.3f");
	EndColumnLabel();
}

void gearbox::componentui::DrawVec3(const std::string& label, Vec3& value, float resetValue, float width)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize))
	{
		value.x = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##X", &value.x, DefaultSpeed);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
	{
		value.y = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &value.y, DefaultSpeed);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
	{
		value.z = resetValue;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Z", &value.z, DefaultSpeed);
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
}

void gearbox::componentui::DrawQuat(const std::string& label, Quat& value, float width)
{
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	float speed = 0.001f;
	double min = -1.0;
	double max = +1.0;

	BeginColumnLabel(label, width);

	ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() - 20.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.4f, 0.4f, 0.45f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.5f, 0.5f, 0.55f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.4f, 0.4f, 0.45f, 1.0f });
	if (ImGui::Button("S", buttonSize))
	{
		value.s = 1.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##S", ImGuiDataType_Double, (void*)&value.s, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("I", buttonSize))
	{
		value.i = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##I", ImGuiDataType_Double, (void*)&value.i, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("J", buttonSize))
	{
		value.j = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##J", ImGuiDataType_Double, (void*)&value.j, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("K", buttonSize))
	{
		value.k = 0.0;
	}
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragScalar("##K", ImGuiDataType_Double, (void*)&value.k, speed, (const void*)&min, (const void*)&max, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndColumnLabel();
}

ComponentSettingsBit gearbox::componentui::DrawComponentSetting()
{
	ComponentSettingsBit result = ComponentSettingsBit::NONE_BIT;

	ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
	if (ImGui::Button(". . ."))
		ImGui::OpenPopup("ComponentSettings");

	bool deleteComponent = false;
	if (ImGui::BeginPopup("ComponentSettings"))
	{
		if (ImGui::MenuItem("Remove"))
			result |= ComponentSettingsBit::REMOVE_BIT;

		ImGui::EndPopup();
	}

	return result;
}
