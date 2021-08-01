#pragma once
#include "gear_core.h"

#include "imgui.h"

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
		UIContext(CreateInfo* pCreateInfo)
			:m_CI(*pCreateInfo)
		{
			Initialise(m_CI.window);
		}
		~UIContext() 
		{
			ShutDown();
		}

		void Initialise(Ref<gear::graphics::Window>& window);
		void NewFrame();
		void RenderDrawData(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, void* drawData);
		void ShutDown();

		inline const CreateInfo& GetCreateInfo() { return m_CI; }

	private:
		VkCommandBuffer GetVkCommandBuffer(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);
		ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList(const Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t index);

		//Members
	private:
		CreateInfo m_CI;

		miru::crossplatform::GraphicsAPI::API m_API;
		VkDescriptorPool m_VulkanDescriptorPool;
		VkDescriptorPoolCreateInfo m_VulkanDescriptorPoolCI;
		ID3D12DescriptorHeap* m_D3D12DescriptorHeapSRV;
	};
}
}