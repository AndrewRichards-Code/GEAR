#pragma once

#include "Graphics/Window.h"

#define GEAR_INPUT_KEYBOARD_AND_MOUSE 0
#define GEAR_INPUT_JOYSTICK_CONTROLLER 1

namespace gear 
{
namespace input 
{
	class InputManager
	{
	private:
		int m_Type;

	public:
		//Joystick Outputs
		bool m_JoyStickPresent = false;
		int m_AxesCount = 0, m_ButtonsCount = 0;
		const float* m_Axis = nullptr;
		const unsigned char* m_Button = nullptr;

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
			std::string joystickName = (glfwGetJoystickName(GLFW_JOYSTICK_1));
			GEAR_PRINTF(("INFO: GEAR::INPUT::InputManager: " + joystickName + ".\n").c_str());

			for (int i = 0; i < m_AxesCount; i++)
				GEAR_PRINTF("INFO: GEAR::INPUT::InputManager: Axis %d: %f1.3.\n", i, m_Axis[i]);

			for (int i = 0; i < m_ButtonsCount; i++)
				GEAR_PRINTF("INFO: GEAR::INPUT::InputManager: Button %d: %c.\n", i, m_Button[i]);

			system("CLS");
		}

	};
}
}