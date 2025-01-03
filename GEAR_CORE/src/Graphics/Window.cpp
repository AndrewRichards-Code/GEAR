#include "gear_core_common.h"
#include "Graphics/Window.h"
#include "Core/Application.h"

#include "ARC/src/StringConversion.h"
#include "stb/stb_image.h"

#include "GLFW/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/include/GLFW/glfw3native.h"

#include <thread>

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace base;

Window::Window(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (!Init())
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

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
}

void Window::Close()
{
	glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
	gear::core::Application::IsActive() = false;
}

bool Window::Closed() const
{
	return glfwWindowShouldClose(m_Window);
}

void Window::CalculateFPS()
{
	double m_CurrentTime = glfwGetTime();
	double m_DeltaTime = m_CurrentTime - m_PreviousTime;
	m_PreviousTime = m_CurrentTime;
	m_FPS = 1.0 / m_DeltaTime;
}

std::string Window::GetGraphicsAPIVersion() const
{
	std::string result("");
	const Context::ResultInfo& resultInfo = m_Context->GetResultInfo();
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::UNKNOWN:
	{
		result += "UNKNOWN"; break;
	}
	case GraphicsAPI::API::D3D12:
	{
		result += "D3D12: ";
		result += "D3D_FEATURE_LEVEL_" + std::to_string(resultInfo.apiVersionMajor)
			+ "_" + std::to_string(resultInfo.apiVersionMinor);
		break;
	}
	case GraphicsAPI::API::VULKAN:
	{
		result += "VULKAN: ";
		result += std::to_string(resultInfo.apiVersionMajor)
			+ "." + std::to_string(resultInfo.apiVersionMinor)
			+ "." + std::to_string(resultInfo.apiVersionPatch);
		break;
	}
	}

	return result;
}

std::string Window::GetDeviceName() const
{
	return m_Context->GetResultInfo().deviceName;
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

void Window::GetScrollPosition(double& position) const
{
	position = m_ScrollPosition;
}
	
bool Window::Init()
{
	if (!glfwInit())
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INIT_FAILED, "Failed to initialise GLFW.");
		return false;
	}

	if (m_CI.applicationContext.GetCommandLineOptions().api == GraphicsAPI::API::VULKAN)
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::NOT_SUPPORTED, "GLFW does not support Vulkan.");
			return false;
		}

		/*bool glfwPresentationSupport = false;
		glfwPresentationSupport = glfwGetPhysicalDevicePresentationSupport(ref_cast<vulkan::Context>(m_Context)->m_Instance, ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDevices[0], 0);
		if (!glfwPresentationSupport)
		{
			GEAR_FATAL(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_NOT_SUPPORTED, "ERROR: GEAR::GRAPHICS::Window: The Vulkan queue family doesn't support presentation to GLFW.");
			return false;
		}*/
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	if (m_CI.fullscreen)
	{
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
		if (monitors && monitorCount > 0 && m_CI.fullscreenMonitorIndex < static_cast<uint32_t>(monitorCount))
			m_Monitor = monitors[m_CI.fullscreenMonitorIndex];
		else
			m_Monitor = glfwGetPrimaryMonitor();

		m_Mode = glfwGetVideoMode(m_Monitor);
		m_CI.width = m_Mode->width;
		m_CI.height = m_Mode->height;
	}
	else
	{
		m_Monitor = glfwGetPrimaryMonitor();
		m_Mode = glfwGetVideoMode(m_Monitor);
	}

	m_Window = glfwCreateWindow((int)m_CI.width, (int)m_CI.height, m_CI.applicationContext.GetCreateInfo().applicationName.c_str(), m_CI.fullscreen ? m_Monitor : nullptr, nullptr);

	if (!m_Window)
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::FUNC_FAILED, "Failed to create GLFW window.");
		return false;
	}

	if (m_CI.maximised)
	{
		glfwMaximizeWindow(m_Window);
	}

	GLFWimage icon[1];
	std::string iconFilepath = !m_CI.iconFilepath.empty() ? m_CI.iconFilepath : "../Branding/GEAR_logo_dark.png";
	icon[0].pixels = stbi_load(iconFilepath.c_str(), &icon->width, &icon->height, 0, 4);
	glfwSetWindowIcon(m_Window, 1, icon);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowCloseCallback(m_Window, window_close);
	glfwSetWindowSizeCallback(m_Window, window_resize);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
	glfwSetCursorPosCallback(m_Window, cursor_position_callback);
	glfwSetJoystickCallback(joystick_callback);
	glfwSetScrollCallback(m_Window, scroll_callback);
	glfwSetDropCallback(m_Window, drop_callback);

	glfwGetWindowSize(m_Window, (int*)&m_CurrentWidth, (int*)&m_CurrentHeight);

	m_Context = m_CI.applicationContext.GetContext();

	Swapchain::CreateInfo swapchainCI;
	swapchainCI.debugName = "GEAR_CORE_Swapchain";
	swapchainCI.context = m_Context;
	swapchainCI.pWindow = glfwGetWin32Window(m_Window);
	swapchainCI.width = m_CurrentWidth;
	swapchainCI.height = m_CurrentHeight;
	swapchainCI.swapchainCount = 2;
	swapchainCI.vSync = m_CI.vSync;
	swapchainCI.bpcColourSpace = Swapchain::BPC_ColourSpace::B8G8R8A8_UNORM_SRGB_NONLINEAR;
	m_Swapchain = Swapchain::Create(&swapchainCI);

	RenderSurface::CreateInfo renderSurfaceCI;
	renderSurfaceCI.debugName = m_CI.applicationContext.GetCreateInfo().applicationName;
	renderSurfaceCI.pContext = m_Context;
	renderSurfaceCI.width = m_CI.width;
	renderSurfaceCI.height = m_CI.height;
	renderSurfaceCI.samples = m_CI.samples;
	m_RenderSurface = CreateRef<RenderSurface>(&renderSurfaceCI);

	return true;
}

void Window::window_close(GLFWwindow* window)
{
	gear::core::Application::IsActive() = false;
}

void Window::window_resize(GLFWwindow* window, int width, int height)
{
	//Handle minimised window state
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	//Handle resizes greater than 4K
	if (width > 3840 || height > 2160)
	{
		GEAR_FATAL(true, "Resize event's dimensions are greater then 4K.");
	}

	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_CurrentWidth = static_cast<uint32_t>(width);
	win->m_CurrentHeight = static_cast<uint32_t>(height);

	win->m_Swapchain->Resize(win->m_CurrentWidth, win->m_CurrentHeight);
	win->m_RenderSurface->Resize(win->m_CurrentWidth, win->m_CurrentHeight);
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
			win->m_Mode = glfwGetVideoMode(win->m_Monitor);
			glfwSetWindowMonitor(win->m_Window, win->m_Monitor, 0, 0, win->m_Mode->width, win->m_Mode->height, win->m_Mode->refreshRate);
			win->m_CurrentWidth = win->m_Mode->width;
			win->m_CurrentHeight = win->m_Mode->height;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (!win->m_Fullscreen)
		{
			glfwSetWindowMonitor(win->m_Window, nullptr, 100, 100, win->m_CI.width, win->m_CI.height, GLFW_DONT_CARE);
			win->m_CurrentWidth = win->m_CI.width;
			win->m_CurrentHeight = win->m_CI.height;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	//Close Window
	if (glfwGetKey(win->m_Window, GLFW_KEY_ESCAPE))
	{
		win->Close();
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
		GEAR_INFO(ErrorCode::CORE | ErrorCode::OK, "The joystick was connected.");
	}
	else if (event == GLFW_DISCONNECTED)
	{
		GEAR_INFO(ErrorCode::CORE | ErrorCode::OK, "The joystick was disconnected.");
	}
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_ScrollPosition += yoffset;
}

void Window::drop_callback(GLFWwindow* window, int pathCount, const char** paths)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	if (win->m_DropCallback)
	{
		std::vector<std::string> _paths;
		for (size_t i = 0; i < pathCount; i++)
		{
			_paths.push_back(paths[i]);
		}
		win->m_DropCallback(_paths);
	}
}