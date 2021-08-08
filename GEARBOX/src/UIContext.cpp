#include "UIContext.h"

#include "vulkan/VKContext.h"
#include "vulkan/VKCommandPoolBuffer.h"
#include "vulkan/VKDescriptorPoolSet.h"
#include "vulkan/VKPipeline.h"

#include "directx12/D3D12Context.h"
#include "directx12/D3D12CommandPoolBuffer.h"
#include "directx12/D3D12Swapchain.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan_with_textures.h"
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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	
	// Setup Descriptor Pool/Heap for ImGui's resources
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		// Initialise ImGui with the GLFW window
		ImGui_ImplGlfw_InitForVulkan(window->GetGLFWwindow(), false);

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
		imGuiVulkanInitInfo.ImageCount = window->GetSwapchain()->GetCreateInfo().swapchainCount;
		ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo, ref_cast<vulkan::RenderPass>(window->GetSwapchainRenderPass())->m_RenderPass);
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		// Initialise ImGui with the GLFW window
		ImGui_ImplGlfw_InitForOther(window->GetGLFWwindow(), false);

		Ref<d3d12::Context> d3d12Context = ref_cast<d3d12::Context>(window->GetContext());

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 100;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT res = d3d12Context->m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_D3D12DescriptorHeapSRV));
		GEAR_ASSERT(res, "GEARBOX: Failed to Create ID3D12DescriptorHeap for ImGui.");

		ImGui_ImplDX12_Init(
			d3d12Context->m_Device, 
			window->GetSwapchain()->GetCreateInfo().swapchainCount,
			ref_cast<d3d12::Swapchain>(window->GetSwapchain())->m_Format,
			m_D3D12DescriptorHeapSRV, 
			m_D3D12DescriptorHeapSRV->GetCPUDescriptorHandleForHeapStart(), 
			m_D3D12DescriptorHeapSRV->GetGPUDescriptorHandleForHeapStart());
	}
	else
	{
		GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

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

void UIContext::BeginFrame()
{
	// Start the Dear ImGui frame for Window and API
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

	// Start the Dear ImGui frame
	ImGui::NewFrame();
}

void UIContext::EndFrame()
{
	// Rendering
	ImGui::Render();
}

void UIContext::RenderDrawData(const Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex, ImDrawData* drawData, UIContext* _this)
{
	if (drawData)
	{
		// Record ImGui commands to the buffer/list
		if (_this->m_API == GraphicsAPI::API::VULKAN)
		{
			ImGui_ImplVulkan_RenderDrawData(reinterpret_cast<ImDrawData*>(drawData), _this->GetVkCommandBuffer(cmdBuffer, frameIndex));
		}
		else if (_this->m_API == GraphicsAPI::API::D3D12)
		{
			ID3D12GraphicsCommandList* cmdList = _this->GetID3D12GraphicsCommandList(cmdBuffer, frameIndex);
			cmdList->SetDescriptorHeaps(1, &(_this->m_D3D12DescriptorHeapSRV));
			ImGui_ImplDX12_RenderDrawData(reinterpret_cast<ImDrawData*>(drawData), cmdList);
		}
		else
		{
			GEAR_ASSERT(gear::ErrorCode::REVERSED | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
		}
	}

	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
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

	// Clean up Descriptor Pool/Heap for ImGui's resources
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

	// Clean up Dear ImGui context
	ImGui::DestroyContext();
}

void UIContext::BeginDockspace()
{
	static bool p_open = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	
	ImGui::Begin("GEARBOX: Dockspace", &p_open, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();
	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("GEARBOX: Dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	DrawMenuBar();
}

void UIContext::EndDockspace()
{
	ImGui::End();
}

VkCommandBuffer UIContext::GetVkCommandBuffer(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return ref_cast<vulkan::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index];
}
ID3D12GraphicsCommandList* UIContext::GetID3D12GraphicsCommandList(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return reinterpret_cast<ID3D12GraphicsCommandList*>(ref_cast<d3d12::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index]);
}

void UIContext::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Close", NULL, false))
			{
				m_CI.window->Close();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}
