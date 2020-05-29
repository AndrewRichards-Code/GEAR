#include "window.h"

#include "directx12/D3D12Context.h"
#include "vulkan/VKContext.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

Window::Window(std::string title, int width, int height, GraphicsAPI::API api, int antiAliasingValue, bool vsync, bool fullscreen)
	:m_Title(title), m_Width(width), m_Height(height), m_AntiAliasingValue(antiAliasingValue), m_VSync(vsync), m_Fullscreen(fullscreen)
{
	m_API = api;
	m_Title += ": GEAR_CORE(x64)";
	
	if (!Init())
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
	m_InitWidth = m_Width;
	m_InitHeight = m_Height;

	for (int i = 0; i < MAX_KEYS; i++)
	{
		m_Keys[i] = false;
	}

	for (int i = 0; i < MAX_BUTTONS; i++)
	{
		m_MouseButtons[i] = false;
	}

	for (int i = 0; i < MAX_JOY_BUTTONS; i++)
	{
		m_JoyButtons[i] = false;
	}
}

Window::~Window()
{
	m_Context->DeviceWaitIdle();
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::Update()
{
	glfwPollEvents();
	//glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
}

bool Window::Closed() const
{
	return glfwWindowShouldClose(m_Window);
}

void Window::Fullscreen()
{
	m_Mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, m_Mode->width, m_Mode->height, m_Mode->refreshRate);
	m_Fullscreen = true;
}

void Window::UpdateVSync(bool vSync)
{
	m_VSync = vSync;
}

void Window::CalculateFPS()
{
	double m_CurrentTime = glfwGetTime();
	double m_DeltaTime = m_CurrentTime - m_PreviousTime;
	m_PreviousTime = m_CurrentTime;
	m_FPS = 1 / m_DeltaTime;
}

std::string Window::GetGraphicsAPIVersion() const
{
	std::string result("");
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::UNKNOWN:
	{ 
		result += "UNKNOWN"; break; 
	}
	case GraphicsAPI::API::D3D12:
	{ 
		result += "D3D12: "; 
		result += "D3D_FEATURE_LEVEL_" + std::to_string(m_ContextCI.api_version_major) 
			+ "_" + std::to_string(m_ContextCI.api_version_minor);
		break; 
	}
	case GraphicsAPI::API::VULKAN:
	{ 
		result += "VULKAN: "; 
		result += std::to_string(m_ContextCI.api_version_major) 
			+ "." + std::to_string(m_ContextCI.api_version_minor);
		break; 
	}
	}

	return result;
}

std::string Window::GetDeviceName() const
{
	std::string result;
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::UNKNOWN:
	{
		result += "UNKNOWN"; break;
	}
	case GraphicsAPI::API::D3D12:
	{
		std::wstring wresult;
		wresult = &ref_cast<d3d12::Context>(m_Context)->m_PhysicalDevices.m_AdapterDescs[0].Description[0];
		result = std::string(wresult.begin(), wresult.end());
		break;
	}
	case GraphicsAPI::API::VULKAN:
	{
		result = &ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDeviceProperties[0].deviceName[0];
		break;
	}
	}

	return result;
}

bool Window::Init()
{
	if (!glfwInit())
	{
		std::cout << "ERROR: GEAR::GRAPHICS::Window: Failed to initialise GLFW!" << std::endl;
		return false;
	}
	miru::GraphicsAPI::SetAPI(m_API);

#ifdef _DEBUG
	GraphicsAPI::LoadGraphicsDebugger();
#endif 

	m_ContextCI.applicationName = m_Title.c_str();
	m_ContextCI.api_version_major = GraphicsAPI::IsD3D12() ? 11 : 1;
	m_ContextCI.api_version_minor = 1;
#ifdef _DEBUG
	m_ContextCI.instanceLayers = { "VK_LAYER_KHRONOS_validation" };
	m_ContextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	m_ContextCI.deviceLayers = { "VK_LAYER_KHRONOS_validation" };
	m_ContextCI.deviceExtensions = { "VK_KHR_swapchain" };
#else
	m_ContextCI.instanceLayers = {};
	m_ContextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	m_ContextCI.deviceLayers = {};
	m_ContextCI.deviceExtensions = { "VK_KHR_swapchain" };
#endif
	m_ContextCI.deviceDebugName = "GEAR_CORE_Context";
	m_Context = Context::Create(&m_ContextCI);

	if (miru::GraphicsAPI::IsVulkan())
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Window: GLFW does not support Vulkan" << std::endl;
			return false;
		}

		bool glfwPresentationSupport = false;
		glfwPresentationSupport = glfwGetPhysicalDevicePresentationSupport(ref_cast<vulkan::Context>(m_Context)->m_Instance, ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDevices[0], 0);
		if (!glfwPresentationSupport)
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Window: The Vulkan queue family doesn't support presentation to GLFW!" << std::endl;
			return false;
		}
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), NULL, NULL);

	if (!m_Window)
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Window: Failed to create GLFW window!" << std::endl;
		return false;
	}
	
	GLFWimage icon[1];
	icon[0].pixels = stbi_load("res/gear_core/GEAR_logo_icon.png", &icon->width, &icon->height, 0, 4);
	glfwSetWindowIcon(m_Window, 1, icon);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowSizeCallback(m_Window, window_resize);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
	glfwSetCursorPosCallback(m_Window, cursor_position_callback);
	glfwSetJoystickCallback(joystick_callback);

	m_SwapchainCI.debugName = "GEAR_CORE_Swapchain";
	m_SwapchainCI.pContext = m_Context;
	m_SwapchainCI.pWindow = glfwGetWin32Window(m_Window);
	m_SwapchainCI.width = m_Width;
	m_SwapchainCI.height = m_Height;
	m_SwapchainCI.swapchainCount = 2;
	m_SwapchainCI.vSync = m_VSync;
	m_Swapchain = Swapchain::Create(&m_SwapchainCI);
	//m_Swapchain->m_Resized = GraphicsAPI::IsVulkan();

	MemoryBlock::CreateInfo dpethMBCI;
	dpethMBCI.debugName = "GEAR_CORE_MB_GPU_SwapchainDepthImage";
	dpethMBCI.pContext = m_Context;
	dpethMBCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_32MB;
	dpethMBCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
	m_DepthMB = MemoryBlock::Create(&dpethMBCI);

	m_DepthImageCI.debugName = "GEAR_CORE_SwapchainDepthImage";
	m_DepthImageCI.device = m_Context->GetDevice();
	m_DepthImageCI.type = Image::Type::TYPE_2D;
	m_DepthImageCI.format = Image::Format::D32_SFLOAT;
	m_DepthImageCI.width = m_Width;
	m_DepthImageCI.height = m_Height;
	m_DepthImageCI.depth = 1;
	m_DepthImageCI.mipLevels = 1;
	m_DepthImageCI.arrayLayers = 1;
	m_DepthImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_DepthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	m_DepthImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
	m_DepthImageCI.size = 0;
	m_DepthImageCI.data = nullptr;
	m_DepthImageCI.pMemoryBlock = m_DepthMB;
	m_DepthImage = Image::Create(&m_DepthImageCI);

	m_DepthImageViewCI.debugName = "GEAR_CORE_SwapchainDepthImageView";
	m_DepthImageViewCI.device = m_Context->GetDevice();
	m_DepthImageViewCI.pImage = m_DepthImage;
	m_DepthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
	m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);

	m_RenderPassCI.debugName = "GEAR_CORE_DefaultRenderPass";
	m_RenderPassCI.device = m_Context->GetDevice();
	m_RenderPassCI.attachments =
	{
		{m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE,RenderPass::AttachmentLoadOp::DONT_CARE, RenderPass::AttachmentStoreOp::DONT_CARE, GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN, Image::Layout::PRESENT_SRC},
		{m_DepthImageCI.format, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::DONT_CARE,RenderPass::AttachmentLoadOp::DONT_CARE, RenderPass::AttachmentStoreOp::DONT_CARE, Image::Layout::UNKNOWN, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
	};
	m_RenderPassCI.subpassDescriptions =
	{
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {}}
	};
	m_RenderPassCI.subpassDependencies =
	{
		{MIRU_SUBPASS_EXTERNAL, 0, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
	};
	m_RenderPass = RenderPass::Create(&m_RenderPassCI);

	CreateFramebuffer();
	return true;
}

void Window::CreateFramebuffer()
{
	m_FramebufferCI.debugName = "GEAR_CORE_DefaultRenderPass";;
	m_FramebufferCI.device = m_Context->GetDevice();;
	m_FramebufferCI.renderPass = m_RenderPass;
	m_FramebufferCI.attachments = {m_Swapchain->m_SwapchainImageViews[0], m_DepthImageView};
	m_FramebufferCI.width = m_Width;
	m_FramebufferCI.height = m_Height;
	m_Framebuffers[0] = Framebuffer::Create(&m_FramebufferCI);
	m_FramebufferCI.attachments = {m_Swapchain->m_SwapchainImageViews[1], m_DepthImageView};
	m_Framebuffers[1] = Framebuffer::Create(&m_FramebufferCI);
}

bool Window::IsKeyPressed(unsigned int keycode) const
{
	if (keycode >= MAX_KEYS)
		return false;
	return m_Keys[keycode];
}

bool Window::IsMouseButtonPressed(unsigned int button) const
{
	if (button >= MAX_BUTTONS)
		return false;
	return m_MouseButtons[button];
}

void Window::GetMousePosition(double& x, double& y) const
{
	x = mx;
	y = my;
}

bool Window::IsJoyButtonPressed(unsigned int button) const
{
	if (button >= MAX_JOY_BUTTONS)
		return false;
	return m_JoyButtons[button];
}

void Window::GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const
{
	x1 = m_xJoy1;
	y1 = m_yJoy1;
	x2 = m_xJoy2;
	y2 = m_yJoy2;
	x3 = m_xJoy3;
	y3 = m_yJoy3;
}
	
void Window::window_resize(GLFWwindow* window, int width, int height)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_Width = width;
	win->m_Height = height;

	win->m_Swapchain->Resize(static_cast<uint32_t>(win->m_Width), static_cast<uint32_t>(win->m_Height));
	if (width <= 3840 && height <= 2160)
	{
		win->m_DepthImageCI.width = win->m_Width;
		win->m_DepthImageCI.height = win->m_Height;
		win->m_DepthImage = Image::Create(&win->m_DepthImageCI);
		win->m_DepthImageViewCI.pImage = win->m_DepthImage;
		win->m_DepthImageView = ImageView::Create(&win->m_DepthImageViewCI);
	}
	win->CreateFramebuffer();
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_Keys[key] = action != GLFW_RELEASE;

	//Maximise Window
	if (glfwGetKey(win->m_Window, GLFW_KEY_F11))
	{
		win->m_Fullscreen = !win->m_Fullscreen;
		if (win->m_Fullscreen)
		{
			win->m_Mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(win->m_Window, glfwGetPrimaryMonitor(), 0, 0, win->m_Mode->width, win->m_Mode->height, win->m_Mode->refreshRate);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (!win->m_Fullscreen)
		{
			glfwSetWindowMonitor(win->m_Window, NULL, 100, 100, win->m_InitWidth, win->m_InitHeight, GL_DONT_CARE);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	//Close Window
	if (glfwGetKey(win->m_Window, GLFW_KEY_ESCAPE))
	{
		win->Closed();
		win->~Window();
		exit(EXIT_SUCCESS);
	}
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_MouseButtons[button] = action != GLFW_RELEASE;
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->mx = xpos;
	win->my = ypos;
}

void Window::joystick_callback(int joy, int event)
{
	if (event == GLFW_CONNECTED)
	{
		std::cout << "The joystick was connected" << std::endl;
	}
	else if (event == GLFW_DISCONNECTED)
	{
		std::cout << "The joystick was disconnected" << std::endl;
	}
}