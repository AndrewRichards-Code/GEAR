#include "gear_core_common.h"
#include "UI/MenuBar.h"
#include "UI/UIContext.h"
#include "Graphics/Window.h"

#include "MIRU/MIRU_CORE/src/vulkan/VKContext.h"
#include "MIRU/MIRU_CORE/src/vulkan/VKCommandPoolBuffer.h"
#include "MIRU/MIRU_CORE/src/vulkan/VKDescriptorPoolSet.h"
#include "MIRU/MIRU_CORE/src/vulkan/VKPipeline.h"

#include "MIRU/MIRU_CORE/src/d3d12/D3D12Context.h"
#include "MIRU/MIRU_CORE/src/d3d12/D3D12CommandPoolBuffer.h"
#include "MIRU/MIRU_CORE/src/d3d12/D3D12Swapchain.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_dx12.h"

using namespace gear;
using namespace ui;
using namespace panels;

using namespace miru;
using namespace base;

UIContext* UIContext::s_UIContext = nullptr;

UIContext::UIContext(CreateInfo* pCreateInfo)
	:m_CI(*pCreateInfo)
{
	s_UIContext = this;
	Initialise(m_CI.window);
}

UIContext::~UIContext()
{
	ShutDown();
}

void UIContext::Update(gear::core::Timer timer)
{
	for (auto& panel : m_EditorPanels)
	{
		panel->Update(timer);
	}

	//Remove old images
	for (auto it = m_TextureIDs.begin(); it != m_TextureIDs.end();)
	{
		if (it->first.use_count() == 1)
			it = m_TextureIDs.erase(it);
		else
			it++;
	}
	//Remove closed panels
	for (auto it = m_EditorPanels.begin(); it != m_EditorPanels.end(); it++)
	{
		Ref<Panel>& panel = *it;
		if (!panel->IsOpen())
		{
			m_EditorPanels.erase(it);
			if (!m_EditorPanels.empty())
				it = m_EditorPanels.begin(); //Reset the iterator.
			else
				break;
		}
	}
}

void UIContext::Draw()
{
	BeginFrame();
	BeginDockspace();
	for (auto& panel : m_EditorPanels)
	{
		panel->Draw();
	}
	EndDockspace();
	EndFrame();
}

void* UIContext::GetDevice()
{
	return m_CI.window->GetDevice();
}

miru::base::ContextRef UIContext::GetContext()
{
	return m_CI.window->GetContext();
}

void UIContext::Initialise(Ref<gear::graphics::Window>& window)
{
	m_API = window->GetApplicationContext().GetCommandLineOptions().api;

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

	std::filesystem::path fontFilepath = std::filesystem::current_path() / std::filesystem::path("res/fonts/electrolize/Electrolize-Regular.ttf");
	io.FontDefault = io.Fonts->AddFontFromFileTTF(fontFilepath.string().c_str(), 15.0f);

	// Setup Dear ImGui style
	SetDarkTheme();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (arc::BitwiseCheck(ImGuiConfigFlags_(io.ConfigFlags), ImGuiConfigFlags_ViewportsEnable))
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	
	// Setup Descriptor Pool/Heap for ImGui's resources
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		// Initialise ImGui with the GLFW window
		ImGui_ImplGlfw_InitForVulkan(window->GetGLFWwindow(), true);

		vulkan::ContextRef vkContext = ref_cast<vulkan::Context>(window->GetContext());
		
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
		VkDescriptorPoolCreateInfo descriptorPoolCI;
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.pNext = nullptr;
		descriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCI.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		descriptorPoolCI.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		descriptorPoolCI.pPoolSizes = poolSizes;
		VkResult res = vkCreateDescriptorPool(vkContext->m_Device, &descriptorPoolCI, nullptr, &m_VulkanDescriptorPool);
		GEAR_FATAL(res, "GEARBOX: Failed to Create VkDescriptorPool for ImGui.");

		//Create Default Sampler
		VkSamplerCreateInfo samplerCI;
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.pNext = nullptr;
		samplerCI.flags = 0;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.anisotropyEnable = false;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareEnable = false;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = 0;
		samplerCI.maxLod = 0;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerCI.unnormalizedCoordinates = false;
		res = vkCreateSampler(vkContext->m_Device, &samplerCI, nullptr, &m_VulkanSampler);
		GEAR_FATAL(res, "GEARBOX: Failed to Create VkDescriptorPool for ImGui.");

		ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {};
		imGuiVulkanInitInfo.Instance = vkContext->m_Instance;
		imGuiVulkanInitInfo.PhysicalDevice = vkContext->m_PhysicalDevices.m_PDIs[0].m_PhysicalDevice;
		imGuiVulkanInitInfo.Device = vkContext->m_Device;
		imGuiVulkanInitInfo.Queue = vkContext->m_Queues[0][0];
		imGuiVulkanInitInfo.DescriptorPool = m_VulkanDescriptorPool;
		imGuiVulkanInitInfo.MinImageCount = 2;
		imGuiVulkanInitInfo.ImageCount = window->GetSwapchain()->GetCreateInfo().swapchainCount;
		imGuiVulkanInitInfo.UseDynamicRendering = true;
		imGuiVulkanInitInfo.ColorAttachmentFormat = static_cast<VkFormat>(window->GetSwapchain()->m_SwapchainImages[0]->GetCreateInfo().format);
		ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo, nullptr);
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		// Initialise ImGui with the GLFW window
		ImGui_ImplGlfw_InitForOther(window->GetGLFWwindow(), true);

		d3d12::ContextRef d3d12Context = ref_cast<d3d12::Context>(window->GetContext());

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1000;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT res = d3d12Context->m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_D3D12DescriptorHeapSRV));
		GEAR_FATAL(res, "GEARBOX: Failed to Create ID3D12DescriptorHeap for ImGui.");

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
		GEAR_FATAL(gear::ErrorCode::UI | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

	// Upload Fonts for Vulkan only
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		CommandPool::CreateInfo cmdPoolCI;
		cmdPoolCI.debugName = "GEARBOX: ImGui: CommandPool";
		cmdPoolCI.context = window->GetContext();
		cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
		CommandPoolRef cmdPool = CommandPool::Create(&cmdPoolCI);

		CommandBuffer::CreateInfo cmdBufferCI;
		cmdBufferCI.debugName = "GEARBOX: ImGui: CommandBuffer";
		cmdBufferCI.commandPool = cmdPool;
		cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmdBufferCI.commandBufferCount = 1;
		CommandBufferRef cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
		
		cmdBuffer->Reset(0, false);
		cmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		ImGui_ImplVulkan_CreateFontsTexture(GetVkCommandBuffer(cmdBuffer, 0));

		cmdBuffer->End(0);

		CommandBuffer::SubmitInfo si = { {0}, {}, {}, {}, {}, {} };
		cmdBuffer->Submit({ si }, nullptr);

		window->GetContext()->DeviceWaitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
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
		GEAR_FATAL(gear::ErrorCode::UI | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}
	ImGui_ImplGlfw_Shutdown();

	// Clean up Descriptor Pool/Heap for ImGui's resources
	if (m_API == GraphicsAPI::API::VULKAN)
	{
		VkDevice device = ref_cast<vulkan::Context>(m_CI.window->GetContext())->m_Device;
		vkDestroySampler(device, m_VulkanSampler, nullptr);
		vkDestroyDescriptorPool(device, m_VulkanDescriptorPool, nullptr);
	}
	else if (m_API == GraphicsAPI::API::D3D12)
	{
		MIRU_D3D12_SAFE_RELEASE(m_D3D12DescriptorHeapSRV);
	}
	else
	{
		GEAR_FATAL(gear::ErrorCode::UI | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

	// Clean up Dear ImGui context
	ImGui::DestroyContext();
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
		GEAR_FATAL(gear::ErrorCode::UI | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
	}

	// Start the Dear ImGui frame
	ImGui::NewFrame();
}

void UIContext::EndFrame()
{
	// Rendering
	ImGui::Render();
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
	if (arc::BitwiseCheck(ImGuiDockNodeFlags_(dockspace_flags), ImGuiDockNodeFlags_PassthruCentralNode))
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
	if (arc::BitwiseCheck(ImGuiConfigFlags_(io.ConfigFlags), ImGuiConfigFlags_DockingEnable))
	{
		ImGuiID dockspace_id = ImGui::GetID("GEARBOX: Dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (!m_MenuBar)
	{
		m_MenuBar = CreateRef<MenuBar>();
	}
	m_MenuBar->Draw();
}

void UIContext::EndDockspace()
{
	ImGui::End();
}

void UIContext::RenderDrawData(const CommandBufferRef& cmdBuffer, uint32_t frameIndex)
{
	ImDrawData* drawData = ImGui::GetDrawData();
	if (drawData)
	{
		// Record ImGui commands to the buffer/list
		if (m_API == GraphicsAPI::API::VULKAN)
		{
			ImGui_ImplVulkan_RenderDrawData(drawData, GetVkCommandBuffer(cmdBuffer, frameIndex));
		}
		else if (m_API == GraphicsAPI::API::D3D12)
		{
			ID3D12GraphicsCommandList* cmdList = GetID3D12GraphicsCommandList(cmdBuffer, frameIndex);
			cmdList->SetDescriptorHeaps(1, &(m_D3D12DescriptorHeapSRV));
			ImGui_ImplDX12_RenderDrawData(drawData, cmdList);

			//Reset previous Descriptor Heaps.
			d3d12::CommandBuffer::RenderingResource& renderingResource = ref_cast<d3d12::CommandBuffer>(cmdBuffer)->m_RenderingResources[frameIndex];
			ID3D12DescriptorHeap* heaps[2] = { renderingResource.CBV_SRV_UAV_DescriptorHeap, renderingResource.SAMPLER_DescriptorHeap };
			cmdList->SetDescriptorHeaps(2, heaps);
		}
		else
		{
			GEAR_FATAL(gear::ErrorCode::UI | gear::ErrorCode::INIT_FAILED, "GEARBOX: Unknown API.");
		}
	}

	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (arc::BitwiseCheck(ImGuiConfigFlags_(io.ConfigFlags), ImGuiConfigFlags_ViewportsEnable))
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

ID3D12GraphicsCommandList* UIContext::GetID3D12GraphicsCommandList(const CommandBufferRef cmdBuffer, uint32_t index)
{
	return reinterpret_cast<ID3D12GraphicsCommandList*>(ref_cast<d3d12::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index]);
}

VkCommandBuffer UIContext::GetVkCommandBuffer(const CommandBufferRef cmdBuffer, uint32_t index)
{
	return ref_cast<vulkan::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index];
}

static ImVec4 operator+(ImVec4& a, ImVec4& b)
{
	return ImVec4(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	);
}

void UIContext::SetDarkTheme()
{
	ImGui::StyleColorsDark();
	ImVec4* colours = ImGui::GetStyle().Colors;

	ImVec4 Grey_Background = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
	ImVec4 Grey_Default = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
	ImVec4 Grey_Hovered = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
	ImVec4 Grey_Active = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
	ImVec4 AddRed = ImVec4(0.8f, 0.0, 0.0, 1.0f);

	colours[ImGuiCol_WindowBg] = Grey_Background;

	// Headers
	colours[ImGuiCol_Header] = Grey_Default;
	colours[ImGuiCol_HeaderHovered] = Grey_Active + AddRed;
	colours[ImGuiCol_HeaderActive] = Grey_Active + AddRed;

	// Buttons
	colours[ImGuiCol_Button] = Grey_Default;
	colours[ImGuiCol_ButtonHovered] = Grey_Active + AddRed;
	colours[ImGuiCol_ButtonActive] = Grey_Active + AddRed;

	// Frame BG
	colours[ImGuiCol_FrameBg] = Grey_Default;
	colours[ImGuiCol_FrameBgHovered] = Grey_Active + AddRed;
	colours[ImGuiCol_FrameBgActive] = Grey_Active + AddRed;

	// Tabs
	colours[ImGuiCol_Tab] = Grey_Active;
	colours[ImGuiCol_TabHovered] = Grey_Active + AddRed;//ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colours[ImGuiCol_TabActive] = Grey_Active + AddRed;// ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colours[ImGuiCol_TabUnfocused] = Grey_Active;
	colours[ImGuiCol_TabUnfocusedActive] = Grey_Default;

	// Title
	colours[ImGuiCol_TitleBg] = Grey_Active;
	colours[ImGuiCol_TitleBgActive] = Grey_Active;
	colours[ImGuiCol_TitleBgCollapsed] = Grey_Active;
}
