#include "gear_core_common.h"
#include "Core/Application.h"

using namespace gear;
using namespace core;

Application* Application::s_Application = nullptr;
bool Application::s_Active = true;

Application::Application(const ApplicationContext& context)
	:m_Context(context), m_VSDebugOutput(CreateScope<arc::VisualStudioDebugOutput>())
{
	s_Application = this;
}

Application::~Application()
{
	s_Application = nullptr;
}
