#include "gear_core_common.h"
#include "Core/Application.h"

using namespace gear;
using namespace core;

Application* Application::s_Application = nullptr;
bool Application::s_Active = true;

Application::Application()
{
	s_Application = this;
}

Application::~Application()
{
	s_Application = nullptr;
}
