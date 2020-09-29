#include "gear_core_common.h"
#include "NativeScriptManager.h"

using namespace gear;
using namespace core;
using namespace scene;

#ifdef _DEBUG
std::string NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path().string() + "\\res\\scripts\\GEAR_NATIVE_SCRIPT\\dll\\x64\\Debug\\";
#else
std::string NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path().string() + "\\res\\scripts\\GEAR_NATIVE_SCRIPT\\dll\\x64\\Release\\";
#endif

void NativeScriptManager::Build()
{
	std::string msBuildDir = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin";
	std::string vcxprojDir = std::filesystem::current_path().string() + "\\res\\scripts\\GEAR_NATIVE_SCRIPT\\";
	std::string solutionDir = std::filesystem::current_path().string() + "\\..\\";

	if (!CheckPath(s_BuildScriptPath) && !CheckPath(msBuildDir) && !CheckPath(vcxprojDir))
		return;
	
	//Build and Link Dynamic Library
	#ifdef _DEBUG
	std::string command = "MSBuild " + vcxprojDir + "GEAR_NATIVE_SCRIPT.vcxproj -t:build /p:Platform=x64 -p:Configuration=Debug -p:SolutionDir=" + solutionDir;
	#else
	std::string command = "MSBuild " + vcxprojDir + " GEAR_NATIVE_SCRIPT.vcxproj -t:build /p:Platform=x64 -p:Configuration=Release -p:SolutionDir=" + solutionDir;
	#endif

	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj using MSBuild\n");
	GEAR_PRINTF(("Executing: " + msBuildDir + "> " + command + "\n").c_str());
	int errorCode = system(("cd " + msBuildDir + " && " + command).c_str());
	GEAR_PRINTF("'MSBuild.exe' has exited with code %d (0x%x).\n", errorCode, errorCode);
	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj finished.\n\n");

	if (!(errorCode == 0))
	{
		system("pause");
		system("cls");
		NativeScriptManager::Build();
	}

	system("cls");
}

DynamicLibrary::LibraryHandle NativeScriptManager::Load()
{
	if (!CheckPath(s_BuildScriptPath))
		return DynamicLibrary::LibraryHandle(0);

	std::string dllFullpath = s_BuildScriptPath + "GEAR_NATIVE_SCRIPT.dll";
	return DynamicLibrary::Load(dllFullpath.c_str());;
}

void NativeScriptManager::Unload(DynamicLibrary::LibraryHandle& libraryHandle)
{
	DynamicLibrary::Unload(libraryHandle);
	libraryHandle = 0;
}

INativeScript* NativeScriptManager::LoadScript(core::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName)
{
	typedef INativeScript* (*PFN_LoadScript)();
	
	INativeScript* ns = nullptr;
	PFN_LoadScript LoadScript = (PFN_LoadScript)DynamicLibrary::LoadFunction(libraryHandle, "LoadScript_" + nativeScriptName);
	if(LoadScript)
		ns = (INativeScript*)LoadScript();
	
	return ns;
}

void NativeScriptManager::UnloadScript(core::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, INativeScript*& nativeScript)
{
	typedef void (*PFN_UnloadScript)(INativeScript*);

	PFN_UnloadScript UnloadScript = (PFN_UnloadScript)DynamicLibrary::LoadFunction(libraryHandle, "UnloadScript_" + nativeScriptName);
	if (UnloadScript)
	{
		UnloadScript(nativeScript);
		nativeScript = nullptr;
	}

}

bool NativeScriptManager::CheckPath(const std::string& directory)
{
	if (!std::filesystem::exists(directory))
	{
		GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::SCENE | core::Log::ErrorCode::INVALID_PATH, "%s does not exist.", s_BuildScriptPath.c_str());
		return false;
	}
	return true;
}
