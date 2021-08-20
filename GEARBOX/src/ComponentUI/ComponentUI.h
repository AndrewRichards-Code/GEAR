#pragma once
#include "gearbox_common.h"

namespace gearbox
{
namespace componentui
{
	constexpr ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
	constexpr float DefaultWidth = 100.0f;
	constexpr float DefaultSpeed = 0.01f;
	constexpr float DefaultMaximumF = FLT_MAX;
	constexpr double DefaultMaximumD = DBL_MAX;

	template<typename T>
	bool DrawTreeNode(const std::string& label, gear::scene::Entity entity)
	{
		if (entity)
		{
			if (entity.HasComponent<T>())
			{
				return ImGui::TreeNodeEx((void*)typeid(T).hash_code(), TreeNodeFlags, label.c_str());
			}
		}
		return false;
	}
	inline void EndDrawTreeNode() { ImGui::TreePop(); }

	inline void BeginColumnLabel(const std::string& label, float width)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, width);
		ImGui::Text(label.c_str());

		ImGui::NextColumn();
	}
	inline void EndColumnLabel()
	{	
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void DrawInputText(const std::string& label, std::string& name, float width = DefaultWidth);
	void DrawCheckbox(const std::string& label, bool& value, float width = DefaultWidth);
	template<typename T>
	void DrawDropDownMenu(const std::string& label, const std::vector<std::string>& items, T& selectedItem, float width = DefaultWidth)
	{
		BeginColumnLabel(label, width);
		if (ImGui::BeginCombo("##label", items[static_cast<size_t>(selectedItem)].c_str()))
		{
			for (size_t i = 0; i < items.size(); i++)
			{
				bool is_selected = (static_cast<size_t>(selectedItem) == i);
				if (ImGui::Selectable(items[i].c_str(), is_selected))
				{
					selectedItem = T(i);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		EndColumnLabel();
	}

	void DrawFloat(const std::string& label, float& value, float minValue, float maxValue = DefaultMaximumF, float width = DefaultWidth);
	void DrawDouble(const std::string& label, double& value, double minValue, double maxValue = DefaultMaximumD, float width = DefaultWidth);
	void DrawVec3(const std::string& label, mars::Vec3& value, float resetValue = 0.0f, float width = DefaultWidth);
	void DrawQuat(const std::string& label, mars::Quat& value, float width = DefaultWidth);

	enum class ComponentSettingsBit : uint64_t
	{
		NONE_BIT = 0x00000000,
		REMOVE_BIT = 0x00000001
	};
	ComponentSettingsBit DrawComponentSetting();

	template<typename T, typename PFN_UI, typename... Args>
	void DrawComponentUI(const std::string& label, gear::scene::Entity entity, PFN_UI pfn_UIFunction, Args&&... args)
	{
		if (entity.HasComponent<T>())
		{
			bool open = DrawTreeNode<T>(label, entity);
			ComponentSettingsBit settings = DrawComponentSetting();
			if (open)
			{
				pfn_UIFunction(std::forward<Args>(args)...);
				EndDrawTreeNode();
			}
			if ((settings & ComponentSettingsBit::REMOVE_BIT) == ComponentSettingsBit::REMOVE_BIT)
			{
				entity.RemoveComponent<T>();
			}
		}
	}
}
}
