#include "gear_core_common.h"
#include "stb_image.h"
#include "Window.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Window::Window(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

#ifdef _DEBUG
	m_CI.title += ": CORE(x64) Debug";
#endif

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
	m_RenderSurface->GetContext()->DeviceWaitIdle();
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::Update()
{
	glfwPollEvents();
}

bool Window::Closed() const
{
	return glfwWindowShouldClose(m_Window);
}

void Window::Fullscreen()
{
	m_Mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, m_Mode->width, m_Mode->height, m_Mode->refreshRate);
}

void Window::CalculateFPS()
{
	double m_CurrentTime = glfwGetTime();
	double m_DeltaTime = m_CurrentTime - m_PreviousTime;
	m_PreviousTime = m_CurrentTime;
	m_FPS = 1.0 / m_DeltaTime;
}

bool Window::Init()
{
	if (!glfwInit())
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INIT_FAILED, "Failed to initialise GLFW.");
		return false;
	}

	if (m_CI.api == GraphicsAPI::API::VULKAN)
	{
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::NOT_SUPPORTED, "GLFW does not support Vulkan.");
			return false;
		}

		/*bool glfwPresentationSupport = false;
		glfwPresentationSupport = glfwGetPhysicalDevicePresentationSupport(ref_cast<vulkan::Context>(m_Context)->m_Instance, ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDevices[0], 0);
		if (!glfwPresentationSupport)
		{
			GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_NOT_SUPPORTED, "ERROR: GEAR::GRAPHICS::Window: The Vulkan queue family doesn't support presentation to GLFW.");
			return false;
		}*/
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_Window = glfwCreateWindow((int)m_CI.width, (int)m_CI.height, m_CI.title.c_str(), NULL, NULL);

	if (!m_Window)
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::FUNC_FAILED, "Failed to create GLFW window.");
		return false;
	}

	m_RenderSurfaceCI.debugName = m_CI.title;
	m_RenderSurfaceCI.window = glfwGetWin32Window(m_Window);
	m_RenderSurfaceCI.api = m_CI.api;
	m_RenderSurfaceCI.width = m_CI.width;
	m_RenderSurfaceCI.height = m_CI.height;
	m_RenderSurfaceCI.vSync = m_CI.vSync;
	m_RenderSurfaceCI.samples = m_CI.samples;
	m_RenderSurfaceCI.graphicsDebugger = m_CI.graphicsDebugger;
	m_RenderSurface = CreateRef<RenderSurface>(&m_RenderSurfaceCI);
	
	GLFWimage icon[1];
	std::string iconFilepath = !m_CI.iconFilepath.empty() ? m_CI.iconFilepath : "../Branding/GEAR_logo_dark.png";
	icon[0].pixels = stbi_load(iconFilepath.c_str(), &icon->width, &icon->height, 0, 4);
	glfwSetWindowIcon(m_Window, 1, icon);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowSizeCallback(m_Window, window_resize);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
	glfwSetCursorPosCallback(m_Window, cursor_position_callback);
	glfwSetJoystickCallback(joystick_callback);
	glfwSetScrollCallback(m_Window, scroll_callback);

	if (m_CI.fullscreen)
		Fullscreen();
	
	return true;
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
	
void Window::window_resize(GLFWwindow* window, int width, int height)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_RenderSurface->Resize(width, height);
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
			win->m_RenderSurface->m_CurrentWidth = win->m_Mode->width;
			win->m_RenderSurface->m_CurrentHeight= win->m_Mode->height;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (!win->m_Fullscreen)
		{
			glfwSetWindowMonitor(win->m_Window, NULL, 100, 100, win->m_CI.width, win->m_CI.height, GLFW_DONT_CARE);
			win->m_RenderSurface->m_CurrentWidth = win->m_CI.width;
			win->m_RenderSurface->m_CurrentHeight = win->m_CI.height;
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
		GEAR_PRINTF("INFO: GEAR::GRAPHICS::Window: The joystick was connected.");
	}
	else if (event == GLFW_DISCONNECTED)
	{
		GEAR_PRINTF("INFO: GEAR::GRAPHICS::Window: The joystick was disconnected.");
	}
}

void Window::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_ScrollPosition += yoffset;
}