#pragma once

#include "../graphics/opengl/window.h"

#define GEAR_INPUT_KEYBOARD_AND_MOUSE 0
#define GEAR_INPUT_JOYSTICK_CONTROLLER 1

namespace GEAR {
namespace INPUT {
class InputManager
{
private:
	int m_Type;

public:
	//Joystick Outputs
	bool m_JoyStickPresent = false;
	int m_AxesCount, m_ButtonsCount;
	const float* m_Axis;
	const unsigned char* m_Button;

public:
	InputManager(int type)
		: m_Type(type) 
	{
		if (m_Type == GEAR_INPUT_KEYBOARD_AND_MOUSE)
		{

		}
		else if (m_Type == GEAR_INPUT_JOYSTICK_CONTROLLER)
		{
			Update();
		}
	}

	~InputManager() {}

	void Update()
	{
		if (m_JoyStickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1))
		{
			m_Axis = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &m_AxesCount);
			m_Button = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &m_ButtonsCount);
		}
	}

	void PrintJoystickDetails()
	{
		std::cout << glfwGetJoystickName(GLFW_JOYSTICK_1) << std::endl;

		for (int i = 0; i < m_AxesCount; i++)
			std::cout << "Axis " << i << ": " << m_Axis[i] << std::endl;

		for (int i = 0; i < m_ButtonsCount; i++)
			std::cout << "Button " << i << ": " << m_Button[i] << std::endl;
		system("CLS");
	}

};
}
}