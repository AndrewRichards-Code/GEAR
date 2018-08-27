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

	bool m_Keys[MAX_KEYS];
	bool m_MouseButtons[MAX_BUTTONS];
	double mx, my;

public:
	Window(std::string title, int width, int height);
	~Window();

	void Clear() const;
	void Update();
	bool Closed() const;
	void Fullscreen();

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline float GetRatio() const { return ((float)m_Width / (float)m_Height); }

	bool IsKeyPressed(unsigned int keycode) const;
	bool IsMouseButtonPressed(unsigned int button) const;
	void GetMousePosition(double& x, double& y) const;

private:
	bool Init();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
};
}
}