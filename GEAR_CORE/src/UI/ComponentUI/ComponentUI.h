#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace scene
	{
		class Scene;
		class Entity;
	}
	namespace ui
	{
		class UIContext;

		namespace componentui
		{
			constexpr ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap;
			constexpr float DefaultWidth = 100.0f;
			constexpr float DefaultSpeed = 0.01f;
			constexpr float DefaultMaximumF = FLT_MAX;
			constexpr double DefaultMaximumD = DBL_MAX;

			inline bool DrawTreeNode(const std::string& label, bool defaultOpen = true, void* ptr_id = nullptr)
			{
				return ImGui::TreeNodeEx((ptr_id ? ptr_id : (void*)label.c_str()), defaultOpen ? (TreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen) : TreeNodeFlags, label.c_str());
			}
			inline void EndDrawTreeNode()
			{
				ImGui::TreePop();
			}

			inline void BeginColumnLabel(const std::string& label, float width)
			{
				ImGui::BeginTable(label.c_str(), 2);

				ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed, width);
				ImGui::TableNextColumn();
				ImGui::Text(label.c_str());

				ImGui::TableNextColumn();
			}
			inline void EndColumnLabel()
			{
				ImGui::EndTable();
			}

			void DrawStaticText(const std::string& label, const std::string& name, float width = DefaultWidth);
			bool DrawInputText(const std::string& label, std::string& name, float width = DefaultWidth);
			bool DrawCheckbox(const std::string& label, bool& value, float width = DefaultWidth);
			template<typename T>
			void DrawDropDownMenu(const std::string& label, T& selectedItem, float width = DefaultWidth)
			{
				constexpr auto items = magic_enum::enum_names<T>();
				BeginColumnLabel(label, width);
				if (ImGui::BeginCombo("##label", std::string(items[static_cast<size_t>(selectedItem)]).c_str()))
				{
					for (size_t i = 0; i < items.size(); i++)
					{
						bool is_selected = (static_cast<size_t>(selectedItem) == i);
						if (ImGui::Selectable(std::string(items[i]).c_str(), is_selected))
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
			template<typename T>
			void DrawDropDownMenu_Bitmask(const std::string& label, const std::vector<std::string>& items, T& selectedItem, float width = DefaultWidth)
			{
				auto GetItemsIndex = [](const T& item) -> size_t
				{
					return static_cast<size_t>(std::bit_width(static_cast<uint32_t>(item)) - 1);
				};

				BeginColumnLabel(label, width);
				if (ImGui::BeginCombo("##label", items[GetItemsIndex(selectedItem)].c_str()))
				{
					for (size_t i = 1; i < (static_cast<size_t>(1) << items.size()); i *= 2)
					{
						bool is_selected = (static_cast<size_t>(selectedItem) == i);
						if (ImGui::Selectable(items[GetItemsIndex(T(i))].c_str(), is_selected))
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
			bool DrawUint32(const std::string& label, uint32_t& value, uint32_t minValue, uint32_t maxValue, bool powerOf2 = false, float width = DefaultWidth, float speed = 1.0f);
			bool DrawInt32(const std::string& label, int32_t& value, int32_t minValue, int32_t maxValue, float width = DefaultWidth, float speed = 1.0f);

			bool DrawFloat(const std::string& label, float& value, float minValue, float maxValue = DefaultMaximumF, float width = DefaultWidth, float speed = DefaultSpeed);
			bool DrawDouble(const std::string& label, double& value, double minValue, double maxValue = DefaultMaximumD, float width = DefaultWidth, float speed = DefaultSpeed);
			bool DrawFloat3(const std::string& label, mars::float3& value, float resetValue = 0.0f, float width = DefaultWidth, float speed = DefaultSpeed);
			bool DrawQuaternion(const std::string& label, mars::Quaternion& value, float width = DefaultWidth, float speed = 0.001f);

			bool DrawColourPicker3(const std::string& label, mars::float3& value, float width = DefaultWidth);
			bool DrawColourPicker4(const std::string& label, mars::float4& value, float width = DefaultWidth);

			void DrawInputVectorOfString(const std::string& label, std::vector<std::string>& vector, float width = DefaultWidth);

			template<typename T>
			void DrawStaticNumber(const std::string& label, const T& value, float width = DefaultWidth)
			{
				BeginColumnLabel(label, width);
				ImGui::Text(std::to_string(value).c_str());
				EndColumnLabel();
			}
			template<typename T>
			void DrawEnum(const std::string& label, const T& selectedItem, float width = DefaultWidth)
			{
				constexpr auto items = magic_enum::enum_names<T>();
				DrawStaticText(label, std::string(items[static_cast<size_t>(selectedItem)]), width);
			}

			enum class ComponentSettingsBit : uint64_t
			{
				NONE_BIT = 0x00000000,
				REMOVE_BIT = 0x00000001
			};
			ComponentSettingsBit DrawComponentSetting();

			template<typename T, typename PFN_UI, typename... Args>
			void DrawComponentUI(const std::string& label, gear::scene::Entity entity, bool defaultOpen, PFN_UI pfn_UIFunction, Args&&... args)
			{
				if (entity.HasComponent<T>())
				{
					bool open = DrawTreeNode(label, defaultOpen);
					ComponentSettingsBit settings = DrawComponentSetting();
					if (open)
					{
						pfn_UIFunction(std::forward<Args>(args)...);
						EndDrawTreeNode();
					}
					if (arc::BitwiseCheck(settings, ComponentSettingsBit::REMOVE_BIT))
					{
						entity.RemoveComponent<T>();
					}
				}
			}

			ImTextureID GetTextureID(const miru::base::ImageViewRef& imageView, gear::ui::UIContext* uiContext, bool resized);
		}
	}
}
