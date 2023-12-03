#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace input
	{
		class GEAR_API InputManager
		{
		public:
			enum class Type
			{
				KEYBOARD_AND_MOUSE,
				JOYSTICK_CONTROLLER,
			};
		private:
			Type m_Type;

		public:
			//Joystick Outputs
			bool m_JoyStickPresent = false;
			int m_AxesCount = 0;
			int m_ButtonsCount = 0;
			const float* m_Axis = nullptr;
			const unsigned char* m_Button = nullptr;

		public:
			InputManager(Type type);
			~InputManager() = default;

			void Update();

			void PrintJoystickDetails();
		};
	}
}