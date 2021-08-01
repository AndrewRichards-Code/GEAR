#include "UIContext.h"

#include "vulkan/VKContext.h"
#include "vulkan/VKCommandPoolBuffer.h"
#include "vulkan/VKDescriptorPoolSet.h"
#include "vulkan/VKPipeline.h"

#include "directx12/D3D12Context.h"
#include "directx12/D3D12CommandPoolBuffer.h"
#include "directx12/D3D12Swapchain.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_dx12.h"

using namespace gearbox;
using namespace imgui;

using namespace miru;
using namespace miru::crossplatform;


void UIContext::Initialise(Ref<gear::graphics::Window>& window)
{
	m_API = window->GetCreateInfo().api;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Initialise ImGui with the GLFW window
	ImGui_ImplGlfw_InitForOther(window->GetGLFWwindow(), false);
	
	// Setup Descriptor Pool/Heap for ImGui's resources
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		Ref<vulkan::Context> vkContext = ref_cast<vulkan::Context>(window->GetContext());
		
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		m_VulkanDescriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		m_VulkanDescriptorPoolCI.pNext = nullptr;
		m_VulkanDescriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		m_VulkanDescriptorPoolCI.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		m_VulkanDescriptorPoolCI.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		m_VulkanDescriptorPoolCI.pPoolSizes = poolSizes;
		VkResult res = vkCreateDescriptorPool(vkContext->m_Device, &m_VulkanDescriptorPoolCI, nullptr, &m_VulkanDescriptorPool);
		GEAR_ASSERT(res, "GEARBOX: Failed to Create VkDescriptorPool for ImGui.");

		ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {};
		imGuiVulkanInitInfo.Instance = vkContext->m_Instance;
		imGuiVulkanInitInfo.PhysicalDevice = vkContext->m_PhysicalDevices.m_PhysicalDevices[0];
		imGuiVulkanInitInfo.Device = vkContext->m_Device;
		imGuiVulkanInitInfo.Queue = vkContext->m_Queues[0][0];
		imGuiVulkanInitInfo.DescriptorPool = m_VulkanDescriptorPool;
		imGuiVulkanInitInfo.MinImageCount = 2;
		imGuiVulkanInitInfo.ImageCount = window->GetRenderSurface()->GetSwapchain()->GetCreateInfo().swapchainCount;
		ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo, ref_cast<vulkan::RenderPass>(window->GetRenderSurface()->GetHDRRenderPass())->m_RenderPass);
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		Ref<d3d12::Context> d3d12Context = ref_cast<d3d12::Context>(window->GetContext());

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT res = d3d12Context->m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_D3D12DescriptorHeapSRV));
		GEAR_ASSERT(res, "GEARBOX: Failed to Create ID3D12DescriptorHeap for ImGui.");

		ImGui_ImplDX12_Init(
			d3d12Context->m_Device, 
			window->GetRenderSurface()->GetSwapchain()->GetCreateInfo().swapchainCount, 
			ref_cast<d3d12::Swapchain>(window->GetRenderSurface()->GetSwapchain())->m_Format,
			m_D3D12DescriptorHeapSRV, 
			m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart(), 
			m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart());
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Upload Fonts for Vulkan only
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		CommandPool::CreateInfo cmdPoolCI;
		cmdPoolCI.debugName = "GEARBOX: ImGui: CommandPool";
		cmdPoolCI.pContext = window->GetContext();
		cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
		Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);

		CommandBuffer::CreateInfo cmdBufferCI;
		cmdBufferCI.debugName = "GEARBOX: ImGui: CommandBuffer";
		cmdBufferCI.pCommandPool = cmdPool;
		cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmdBufferCI.commandBufferCount = 1;
		Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
		
		cmdBuffer->Reset(0, false);
		cmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		ImGui_ImplVulkan_CreateFontsTexture(GetVkCommandBuffer(cmdBuffer, 0));

		cmdBuffer->End(0);
		cmdBuffer->Submit({ 0 }, {}, {}, {}, nullptr);

		window->GetContext()->DeviceWaitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void UIContext::NewFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplGlfw_NewFrame();
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		ImGui_ImplVulkan_NewFrame();
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		ImGui_ImplDX12_NewFrame();
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}
}

void UIContext::RenderDrawData(const Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex, void* drawData)
{
	if (!drawData)
		return;

	if (m_API == GraphicsAPI::API::VULKAN)
	{
		ImGui_ImplVulkan_RenderDrawData(reinterpret_cast<ImDrawData*>(drawData), GetVkCommandBuffer(cmdBuffer, frameIndex));
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		ID3D12GraphicsCommandList* cmdList = GetID3D12GraphicsCommandList(cmdBuffer, frameIndex);
		cmdList->SetDescriptorHeaps(1, &m_D3D12DescriptorHeapSRV);
		ImGui_ImplDX12_RenderDrawData(reinterpret_cast<ImDrawData*>(drawData), cmdList);
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}
}

void UIContext::ShutDown()
{
	// Start the Dear ImGui frame
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		ImGui_ImplVulkan_Shutdown();
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		ImGui_ImplDX12_Shutdown();
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}
	ImGui_ImplGlfw_Shutdown();

	if (m_API == GraphicsAPI::API::VULKAN)
	{
		VkDevice device = ref_cast<vulkan::Context>(m_CI.window->GetContext())->m_Device;
		vkDestroyDescriptorPool(device, m_VulkanDescriptorPool, nullptr);
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		MIRU_D3D12_SAFE_RELEASE(m_D3D12DescriptorHeapSRV);
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

	ImGui::DestroyContext();
}

VkCommandBuffer UIContext::GetVkCommandBuffer(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return ref_cast<vulkan::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index];
}
ID3D12GraphicsCommandList* UIContext::GetID3D12GraphicsCommandList(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return reinterpret_cast<ID3D12GraphicsCommandList*>(ref_cast<d3d12::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index]);
}