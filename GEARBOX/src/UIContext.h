#pragma once
#include "gearbox_common.h"
#include "Panels/Panel.h"

namespace gearbox
{

namespace panels
{
	class ViewportPanel;
	class SceneHierarchyPanel;
	class PropertiesPanel;
	class ContentBrowserPanel;
	class ContentEditorPanel;
}

namespace imgui
{
	class UIContext final
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

		void Draw();

		inline const CreateInfo& GetCreateInfo() { return m_CI; }
		inline std::vector<Ref<panels::Panel>>& GetEditorPanels() { return m_EditorPanels; };

		template<typename T>
		Ref<T> GetPanel()
		{
			Ref<panels::Panel> selectedPanel = nullptr;
			for (auto& panel : m_EditorPanels)
			{
				if (panel->GetPanelType() == panels::Panel::Type::UNKNOWN)
				{
					continue;
				}
				if (typeid(T) == typeid(panels::ViewportPanel) && panel->GetPanelType() == panels::Panel::Type::VIEWPORT)
				{
					selectedPanel = panel;
					break;
				}
				if (typeid(T) == typeid(panels::SceneHierarchyPanel) && panel->GetPanelType() == panels::Panel::Type::SCENE_HIERARCHY)
				{
					selectedPanel = panel;
					break;
				}
				if (typeid(T) == typeid(panels::PropertiesPanel) && panel->GetPanelType() == panels::Panel::Type::PROPERTIES)
				{
					selectedPanel = panel;
					break;
				}
				if (typeid(T) == typeid(panels::ContentBrowserPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_BROWSER)
				{
					selectedPanel = panel;
					break;
				}
				if (typeid(T) == typeid(panels::ContentEditorPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_EDITOR)
				{
					selectedPanel = panel;
					break;
				}
			}

			return ref_cast<T>(selectedPanel);
		}

	private:
		void Initialise(Ref<gear::graphics::Window>& window);
		void ShutDown();

		void BeginFrame();
		void EndFrame();

		void BeginDockspace();
		void EndDockspace();
	
	public:
		static void RenderDrawData(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, ImDrawData* drawData,  UIContext* _this);
	
	private:
		bool ShortcutPressed(const std::vector<uint32_t>& keycodes);
		void DrawMenuBar();
		void SetDarkTheme();

		ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);
		VkCommandBuffer GetVkCommandBuffer(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);

		//Members
	private:
		CreateInfo m_CI;

		miru::crossplatform::GraphicsAPI::API m_API;
		std::vector<Ref<panels::Panel>> m_EditorPanels;

	public:
		VkDescriptorPool m_VulkanDescriptorPool;
		VkDescriptorPoolCreateInfo m_VulkanDescriptorPoolCI;
		VkSampler m_VKSampler = VK_NULL_HANDLE;
	
		ID3D12DescriptorHeap* m_D3D12DescriptorHeapSRV;
		std::map<ImTextureID, UINT> m_D3D12GPUHandleHeapOffsets;
		UINT m_GPUHandleHeapIndex = 1;
	};
}
}