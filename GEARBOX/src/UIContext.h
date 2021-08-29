#pragma once
#include "gearbox_common.h"
#include "Panels/Panel.h"

namespace gearbox
{
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
		void DrawMenuBar();
		void SetDarkTheme();

		ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);
		VkCommandBuffer GetVkCommandBuffer(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);

		//Members
	private:
		CreateInfo m_CI;

		miru::crossplatform::GraphicsAPI::API m_API;
	public:
		VkDescriptorPool m_VulkanDescriptorPool;
		VkDescriptorPoolCreateInfo m_VulkanDescriptorPoolCI;
		VkSampler m_VKSampler = VK_NULL_HANDLE;
	
		ID3D12DescriptorHeap* m_D3D12DescriptorHeapSRV;
		std::map<ImTextureID, UINT> m_D3D12GPUHandleHeapOffsets;
		UINT m_GPUHandleHeapIndex = 1;

		std::vector<Ref<panels::Panel>> editorPanels;
		std::vector<Ref<panels::Panel>>& GetEditorPanels() { return editorPanels; };
	};
}
}