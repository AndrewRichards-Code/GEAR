#include "Input/InputManager.h"
#include "GLFW/include/GLFW/glfw3.h"

using namespace gear;
using namespace input;
	
InputManager::InputManager(Type type)
	: m_Type(type)
{
	if (m_Type == Type::KEYBOARD_AND_MOUSE)
	{

	}
	else if (m_Type == Type::JOYSTICK_CONTROLLER)
	{
		Update();
	}
}


void InputManager::Update()
{
	if (m_JoyStickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1))
	{
		m_Axis = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &m_AxesCount);
		m_Button = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &m_ButtonsCount);
	}
}

void InputManager::PrintJoystickDetails()
{
	std::string joystickName = (glfwGetJoystickName(GLFW_JOYSTICK_1));
	GEAR_INFO(ErrorCode::CORE | ErrorCode::OK, joystickName.c_str());

	for (int i = 0; i < m_AxesCount; i++)
		GEAR_INFO(ErrorCode::CORE | ErrorCode::OK, "Axis %d: %f1.3.\n", i, m_Axis[i]);

	for (int i = 0; i < m_ButtonsCount; i++)
		GEAR_INFO(ErrorCode::CORE | ErrorCode::OK, "Button %d: %c.\n", i, m_Button[i]);

	system("CLS");
}
