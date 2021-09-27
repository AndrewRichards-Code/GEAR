#include "gearbox_common.h"
#include "UIContext.h"
#include "Panels/Panels.h"
#include "Core/FileDialog.h"

#include "vulkan/VKContext.h"
#include "vulkan/VKCommandPoolBuffer.h"
#include "vulkan/VKDescriptorPoolSet.h"
#include "vulkan/VKPipeline.h"

#include "directx12/D3D12Context.h"
#include "directx12/D3D12CommandPoolBuffer.h"
#include "directx12/D3D12Swapchain.h"

using namespace gearbox;
using namespace imgui;
using namespace panels;

using namespace miru;
using namespace miru::crossplatform;

UIContext::UIContext(CreateInfo* pCreateInfo)
	:m_CI(*pCreateInfo)
{
	Initialise(m_CI.window);
}

UIContext::~UIContext()
{
	ShutDown();
}

void UIContext::Draw()
{
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

	BeginFrame();
	BeginDockspace();
	for (auto& panel : m_EditorPanels)
	{
		panel->Draw();
	}
	EndDockspace();
	EndFrame();
}

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

	io.FontDefault = io.Fonts->AddFontFromFileTTF("res/fonts/electrolize/Electrolize-Regular.ttf", 15.0f);

	// Setup Dear ImGui style
	SetDarkTheme();

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
		ImGui_ImplGlfw_InitForVulkan(window->GetGLFWwindow(), true);

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
		ImGui_ImplGlfw_InitForOther(window->GetGLFWwindow(), true);

		Ref<d3d12::Context> d3d12Context = ref_cast<d3d12::Context>(window->GetContext());

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1000;
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

ID3D12GraphicsCommandList* UIContext::GetID3D12GraphicsCommandList(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return reinterpret_cast<ID3D12GraphicsCommandList*>(ref_cast<d3d12::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index]);
}

VkCommandBuffer UIContext::GetVkCommandBuffer(const Ref<CommandBuffer> cmdBuffer, uint32_t index)
{
	return ref_cast<vulkan::CommandBuffer>(cmdBuffer)->m_CmdBuffers[index];
}

bool UIContext::ShortcutPressed(const std::vector<uint32_t>& keycodes)
{
	m_CI.window->Update();

	bool result = true;
	for (const auto& keycode : keycodes)
	{
		result &= m_CI.window->IsKeyPressed(static_cast<unsigned int>(keycode));
	}
	return result;
}

void UIContext::DrawMenuBar()
{
	static std::function<void()> PopupWindow = nullptr;
	bool saveToSaveAs = false;

	auto NewScene = [&]()
	{
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel)
		{
			gear::scene::Scene::CreateInfo sceneCI;
			sceneCI.debugName = "DefaultScene";
			sceneCI.nativeScriptDir = "res/scripts/";
			sceneHierarchyPanel->SetScene(CreateRef<gear::scene::Scene>(&sceneCI));
			sceneHierarchyPanel->UpdateWindowTitle();
		}
	};
	auto OpenScene = [&]()
	{
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel)
		{
			void* window = (void*)glfwGetWin32Window(m_CI.window->GetGLFWwindow());
			std::string filepath = gear::core::FileDialog_Open(window, "GEAR Scene file (*.gsf)\0*.gsf\0");

			if (!filepath.empty())
			{
				sceneHierarchyPanel->GetScene()->LoadFromFile(filepath, m_CI.window);
				sceneHierarchyPanel->UpdateWindowTitle();
			}
		}
	};
	auto SaveScene = [&]()
	{
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel)
		{
			if (!sceneHierarchyPanel->GetScene()->GetFilepath().empty())
			{
				sceneHierarchyPanel->GetScene()->SaveToFile("");
				sceneHierarchyPanel->UpdateWindowTitle();
			}
			else
			{
				saveToSaveAs = true;
			}
		}
	};
	auto SaveSceneAs = [&]()
	{
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel)
		{
			void* window = (void*)glfwGetWin32Window(m_CI.window->GetGLFWwindow());
			std::string filepath = gear::core::FileDialog_Save(window, "GEAR Scene file (*.gsf)\0*.gsf\0");

			if (!filepath.empty())
			{
				sceneHierarchyPanel->GetScene()->SaveToFile(filepath);
				sceneHierarchyPanel->UpdateWindowTitle();
			}
		}
	};
	auto Exit = [&]()
	{
		m_CI.window->Close();
	};
	auto About = [&]()
	{
		if (!ImGui::IsPopupOpen("Version"))
			ImGui::OpenPopup("Version");

		if (ImGui::BeginPopupModal("Version"))
		{
			ImGui::Text("GEARBOX - Version: %d.%d.%d %s", GEAR_VERSION_MAJOR, GEAR_VERSION_MINOR, GEAR_VERSION_PATCH, GEAR_VERSION_TYPE_STR);
			std::string gpu = "GPU: " + m_CI.window->GetDeviceName();
			ImGui::Text(gpu.c_str());
			std::string api = "API: " + m_CI.window->GetGraphicsAPIVersion();
			ImGui::Text(api.c_str());
			if (ImGui::Button("Close", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
				PopupWindow = nullptr;
			}
			ImGui::EndPopup();
		}
	};

	
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_N }))
		NewScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_O }))
		OpenScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_S }))
		SaveScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_S }) || saveToSaveAs)
		SaveSceneAs();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				NewScene();
			if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
				OpenScene();
			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				SaveScene();
			if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S") || saveToSaveAs)
				SaveScene();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
				Exit();
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About", nullptr, false))
			{
				PopupWindow = About;
			}
			ImGui::EndMenu();
		}
		
		ImGui::EndMenuBar();
	}

	if (PopupWindow)
		PopupWindow();
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
