#include "window.h"
#include "stb_image.h"

using namespace GEAR;
using namespace GRAPHICS;

void window_resize(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

Window::Window(std::string title, int width, int height)
	:m_Title(title), m_Width(width), m_Height(height)
{
#ifdef _M_X64
	m_Title += ": GEAR_CORE(x64-86)";
#elif _M_IX86
	m_Title += ": GEAR_CORE(x86)";
#endif
	if (!Init())
		glfwTerminate();
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
}

Window::~Window()
{
	//TODO: Fix this atioglxx.dll error
	//glfwTerminate();
}

void Window::Clear() const
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::Update()
{
	glfwPollEvents();
	glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
	glfwSwapBuffers(m_Window);
}

bool Window::Closed() const
{
	return glfwWindowShouldClose(m_Window);
}

void Window::Fullscreen()
{
	m_Mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, m_Mode->width, m_Mode->height, m_Mode->refreshRate);
	glfwSwapInterval(1);
	m_Fullscreen = true;
}

bool Window::Init()
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialise GLFW!" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), NULL, NULL);

	if (!m_Window)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW window!" << std::endl;
		return 1;
	}
	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1);
	
	GLFWimage icon[1];
	icon[0].pixels = stbi_load("res/gear_core/GEAR_logo_icon.png", &icon->width, &icon->height, 0, 4);
	glfwSetWindowIcon(m_Window, 1, icon);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowSizeCallback(m_Window, window_resize);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
	glfwSetCursorPosCallback(m_Window, cursor_position_callback);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialise GLEW!" << std::endl;
		return false;
	}
	std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
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

void Window::GetMousePosition(double & x, double & y) const
{
	x = mx;
	y = my;
}

void window_resize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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
			glfwSwapInterval(1);
		}

		if (!win->m_Fullscreen)
		{
			glfwSetWindowMonitor(win->m_Window, NULL, 100, 100, win->m_InitWidth, win->m_InitHeight, GL_DONT_CARE);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			glfwSwapInterval(1);
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

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_MouseButtons[button] = action != GLFW_RELEASE;
}

void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->mx = xpos;
	win->my = ypos;
}
