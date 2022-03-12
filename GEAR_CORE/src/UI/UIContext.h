#pragma once
#include "gear_core_common.h"
#include "Panels/BasePanel.h"

#include "Core/Timer.h"
#include "Build/Project.h"
#include "Graphics/Window.h"

namespace gear
{
namespace ui
{
	namespace panels
	{
		class ContentBrowserPanel;
		class ContentEditorPanel;
		class SceneHierarchyPanel;
		class MaterialPanel;
		class ProjectPanel;
		class PropertiesPanel;
		class ViewportPanel;
	}
	class MenuBar;

	class GEAR_API UIContext
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			Ref<gear::graphics::Window> window;
		};

		//Methods
	public:
		UIContext(CreateInfo* pCreateInfo);
		~UIContext();

		void Update(gear::core::Timer timer);
		void Draw();

		inline void* GetDevice() { return m_CI.window->GetDevice(); }
		inline Ref<miru::crossplatform::Context> GetContext() { return m_CI.window->GetContext(); }
		inline Ref<gear::graphics::Window> GetWindow() { return m_CI.window; }
		inline const CreateInfo& GetCreateInfo() { return m_CI; }
		inline std::vector<Ref<panels::Panel>>& GetEditorPanels() { return m_EditorPanels; };
		inline Ref<build::Project> GetProject() { return m_Project; }
		inline void SetProject(const Ref<build::Project>& project) { m_Project = project; }

		template<typename T>
		std::vector<Ref<T>> GetEditorPanelsByType()
		{
			std::vector<Ref<panels::Panel>> selectedPanels;
			for (auto& panel : m_EditorPanels)
			{
				if (panel->GetPanelType() == panels::Panel::Type::UNKNOWN)
				{
					continue;
				}
				if (typeid(T) == typeid(panels::ContentBrowserPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_BROWSER)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::ContentEditorPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_EDITOR)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::SceneHierarchyPanel) && panel->GetPanelType() == panels::Panel::Type::SCENE_HIERARCHY)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::MaterialPanel) && panel->GetPanelType() == panels::Panel::Type::MATERIAL)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::ProjectPanel) && panel->GetPanelType() == panels::Panel::Type::PROJECT)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::PropertiesPanel) && panel->GetPanelType() == panels::Panel::Type::PROPERTIES)
				{
					selectedPanels.push_back(panel);
					continue;
				}
				if (typeid(T) == typeid(panels::ViewportPanel) && panel->GetPanelType() == panels::Panel::Type::VIEWPORT)
				{
					selectedPanels.push_back(panel);
					continue;
				}
			}

			if(selectedPanels.empty())
				selectedPanels.push_back(nullptr);

			std::vector<Ref<T>> _selectedPanels;
			for (auto& panel : selectedPanels)
				_selectedPanels.push_back(ref_cast<T>(panel));

			return _selectedPanels;
		}

		template<typename T>
		std::string GetUniqueIDString(const std::string& label, T* _this)
		{
			size_t idx = 0;
			for (auto& panel : GetEditorPanelsByType<T>())
			{
				if (panel.get() == _this)
					break;

				idx++;
			}
			size_t structHash = typeid(T).hash_code();
			std::string uid_str = label + std::string("##") + std::to_string(structHash) + "_" + std::to_string(idx);
			return uid_str;
		}

	private:
		void Initialise(Ref<gear::graphics::Window>& window);
		void ShutDown();

		void BeginFrame();
		void EndFrame();

		void BeginDockspace();
		void EndDockspace();
	
	public:
		void RenderDrawData(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex);
		static UIContext* GetUIContext() { return s_UIContext; }
	
	private:
		void SetDarkTheme();

		ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);
		VkCommandBuffer GetVkCommandBuffer(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);

		//Members
	private:
		CreateInfo m_CI;

		miru::crossplatform::GraphicsAPI::API m_API;
		std::vector<Ref<panels::Panel>> m_EditorPanels;
		Ref<MenuBar> m_MenuBar;
		Ref<gear::build::Project> m_Project;

		static UIContext* s_UIContext;

	public:
		VkDescriptorPool m_VulkanDescriptorPool;
		VkDescriptorPoolCreateInfo m_VulkanDescriptorPoolCI;
		VkSampler m_VulkanSampler;
		VkSamplerCreateInfo m_VulkanSamplerCI;
	
		ID3D12DescriptorHeap* m_D3D12DescriptorHeapSRV;
		std::map<ImTextureID, UINT> m_D3D12GPUHandleHeapOffsets;
		UINT m_GPUHandleHeapIndex = 1;

		std::map<Ref<miru::crossplatform::ImageView>, ImTextureID> m_TextureIDs;
	};
}
}