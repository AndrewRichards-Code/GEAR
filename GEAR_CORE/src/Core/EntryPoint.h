#pragma once
#include "Application.h"

#ifndef GEAR_EXTERNAL_ENTRY_POINT

extern Ref<gear::core::Application> CreateApplication(int argc, char** argv);

int main(int argc, char** argv) 
{
	Ref<gear::core::Application> app = CreateApplication(argc, argv);
	if (!app)
	{
		GEAR_ASSERT(gear::ErrorCode::CORE | gear::ErrorCode::INIT_FAILED, "Failed to Create user-defined Application.");
	}
	app->Run();
}

#endif