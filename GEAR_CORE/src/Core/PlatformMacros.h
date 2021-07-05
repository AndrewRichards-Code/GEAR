#pragma once

#pragma region GEAR_GENERAL_PLATFORM_MACROS
#if defined(_WIN64)
	#if defined(_M_ARM64)
		//Windows ARM64
		#define	GEAR_PLATFORM_WINDOWS_ARM64
	#else
		//Windows x64
		#define GEAR_PLATFORM_WINDOWS_X64
		#if defined(_GAMING_DESKTOP)
			//GDK: Gaming.Desktop.x64
			#define GEAR_PLATFORM_GAMING_DESKTOP_X64
		#endif
	#endif
#elif defined(__APPLE__)
	//Mac OS
	#define GEAR_PLATFORM_MAC_OS
	//iOS
	#define GEAR_PLATFORM_IOS
#elif defined(__linux__)
	//Linux
	#define GEAR_PLATFORM_LINUX
	#if defined(__ANDROID__)
		//Android
		#define GEAR_PLATFORM_ANDROID
	#endif
#else
	#error "GEAR_CORE: Unknown Platform."
#endif
#pragma endregion

#pragma region GEAR_XBOX_MACROS
#if defined(XBOX) || defined(_XBOX_ONE) || defined(_DURANGO)
	//XDK: XboxOne
	#define GEAR_PLATFORM_XBOX_ONE_XDK
#elif defined(_GAMING_XBOX) 
	//GDKX: Gaming.Xbox.XboxOne.x64
	#define GEAR_PLATFORM_XBOX_ONE_GDKX
#elif defined(_GAMING_XBOX_SCARLETT)
	//GDKX: Gaming.Xbox.Scarlett.x64
	#define GEAR_PLATFORM_XBOX_SCARLETT
#endif
#pragma endregion

#pragma region GEAR_WINDOWS_WINAPI_MACROS
#if !defined(WINAPI_FAMILY)
	#include <winapifamily.h>
#endif
#if defined(WINAPI_FAMILY)
	#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
		//Windows WIN32 API
		#define GEAR_PLATFORM_WINDOWS_FAMILY_DESKTOP
	#elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
		//Windows UWP API
		#define GEAR_PLATFORM_WINDOWS_FAMILY_UWP
	#elif WINAPI_FAMILY == WINAPI_FAMILY_GAMES
		#define GEAR_PLATFORM_WINDOWS_FAMILY_GAMES
	#endif
#endif
#pragma endregion

#pragma region GEAR_DEFINED_PLATFORM_MACROS
#if defined(GEAR_PLATFORM_XBOX_ONE_XDK) || defined(GEAR_PLATFORM_XBOX_ONE_GDKX) || defined(GEAR_PLATFORM_XBOX_SCARLETT)
	#define GEAR_PLATFORM_XBOX_X64
#endif

#if defined (GEAR_PLATFORM_WINDOWS_X64) || defined(GEAR_PLATFORM_XBOX_X64)
	#define GEAR_PLATFORM_WINDOWS_OR_XBOX 
#endif
#pragma endregion