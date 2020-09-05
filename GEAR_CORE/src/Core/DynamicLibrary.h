#pragma once

#include "gear_core_common.h"

namespace gear
{
namespace core
{
	class DynamicLibrary
	{
	public:
		#if defined(_WIN64)
		typedef HMODULE LibraryHandle;
		#elif defined(__linux__)
		typedef void* LibraryHandle;
		#endif

		typedef void* PFN_LibraryFunction;

		static LibraryHandle Load(const std::string& libraryFilepath)
		{
			LibraryHandle libraryHandle = 0;

			#if defined(_WIN64)
			libraryHandle = LoadLibraryA(libraryFilepath.c_str());
			#elif defined(__linux__)
			libraryHandle = dlopen(libraryFilepath.c_str(), RTLD_NOW | RTLD_NOLOAD);
			#endif
			
			if (!libraryHandle)
			{
				GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::CORE | core::Log::ErrorCode::NO_FILE, "%s does not exist. Error: %d", libraryFilepath.c_str(), GetLastError());
			}
			return libraryHandle;
		}

		static void Unload(LibraryHandle& libraryHandle)
		{
			bool success = false;
			#if defined(_WIN64)
			success = FreeLibrary(libraryHandle);
			#elif defined(__linux__)
			libraryHandle = dlclose(libraryHandle);
			#endif

			if (!success)
			{
				GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::CORE | core::Log::ErrorCode::INVALID_VALUE, "0x%x is not valid. Error: %d", libraryHandle, GetLastError());
			}
		}

		static PFN_LibraryFunction LoadFunction(LibraryHandle libraryHandle, const std::string& functionName)
		{
			PFN_LibraryFunction pfn = nullptr;
			#if defined(_WIN64)
			pfn = GetProcAddress(libraryHandle, functionName.c_str());
			#elif defined(__linux__)
			pfn = dlsym(libraryHandle, functionName.c_str());
			#endif

			if (!pfn)
			{
				GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::CORE | core::Log::ErrorCode::NO_FILE, "Can not load function: %s", functionName.c_str());
			}
			return pfn;
		}
	};
}
}