#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

namespace GEAR {
namespace GRAPHICS
{

#define MAX_KEYS 1024 
#define MAX_BUTTONS 32 
#define MAX_AXES 6
#define MAX_JOY_BUTTONS 14

class Window
{
private:
	friend struct GLFWwindow;

	std::string m_Title;
	int m_Width, m_Height;
	int m_InitWidth, m_InitHeight;
	GLFWwindow* m_Window;
	const GLFWvidmode* m_Mode;
	bool m_Fullscreen = false;
	bool m_VSync = true;
	int m_AntiAliasingValue = 0;

	double m_CurrentTime, m_PreviousTime = 0.0, m_DeltaTime, m_FPS;

	bool m_Keys[MAX_KEYS];
	bool m_MouseButtons[MAX_BUTTONS];
	double mx, my;

	bool m_JoyButtons[MAX_JOY_BUTTONS];
	double m_xJoy1, m_yJoy1, m_xJoy2, m_yJoy2, m_xJoy3, m_yJoy3;

public:
	Window(std::string title, int width, int height, int antiAliasingValue);
	virtual ~Window();

	void Clear() const;
	void Update();
	bool Closed() const;
	void Fullscreen();
	void UpdateVSync(bool vSync);
	void CalculateFPS();
	
	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline float GetRatio() const { return ((float)m_Width / (float)m_Height); }
	inline std::string GetTitle() const { return m_Title; }
	inline std::string GetOpenGLVersion() const { return std::string((const char*)glGetString(GL_VERSION)); }
	inline std::string GetFPSString() const { return std::to_string(m_FPS); }
	inline std::string GetAntiAliasingValue() const { return std::to_string(m_AntiAliasingValue); }

	bool IsKeyPressed(unsigned int keycode) const;
	bool IsMouseButtonPressed(unsigned int button) const;
	void GetMousePosition(double& x, double& y) const;
	bool IsJoyButtonPressed(unsigned int button) const;
	void GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const;

private:
	bool Init();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void joystick_callback(int joy, int event);
};
}
}