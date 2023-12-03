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
			GEAR_ASSERT(gear::ErrorCode::CORE | gear::ErrorCode::INIT_FAILED, "Failed to Create user-defined Application.");
		}
		app->Run();
	}

	return 0;
}

int main(int argc, char** argv)
{
	return GEAR_main(argc, argv);
}
