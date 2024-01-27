#pragma once
#include "Core/PlatformMacros.h"

extern Ref<gear::core::Application> CreateApplication(int argc, char** argv);

int GEAR_main(int argc, char** argv)
{
	while (gear::core::Application::IsActive())
	{		
		Ref<gear::core::Application> app = CreateApplication(argc, argv);
		if (!app)
		{
			GEAR_FATAL(gear::ErrorCode::CORE | gear::ErrorCode::INIT_FAILED, "Failed to Create user-defined Application.");
		}
		app->Run();
	}

	return 0;
}

#if defined(GEAR_PLATFORM_WINDOWS_X64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return GEAR_main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return GEAR_main(argc, argv);
}

#endif