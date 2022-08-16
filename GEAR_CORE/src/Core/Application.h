#pragma once
#include "gear_core_common.h"
#include "Core/ApplicationContext.h"

namespace gear
{
	namespace core
	{
		class GEAR_API Application
		{
		public:
			Application(const ApplicationContext& context);
			virtual ~Application();

			virtual void Run() = 0;

			static Application* GetApplication() { return s_Application; }
			static bool& IsActive() { return s_Active; }

		protected:
			ApplicationContext m_Context;

		private:
			static Application* s_Application;
			static bool s_Active;
		};
	}
}