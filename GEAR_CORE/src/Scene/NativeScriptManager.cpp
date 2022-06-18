#include "gear_core_common.h"
#include "Scene/NativeScriptManager.h"

using namespace arc;
using namespace gear;
using namespace scene;

#ifdef _DEBUG
std::string NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path().string() + "\\..\\bin\\x64\\Debug\\";
#else
std::string NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path().string() + "\\..\\bin\\x64\\Release\\";
#endif

void NativeScriptManager::Build(const std::string& nativeScriptDir)
{
	std::string msBuildDir = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Msbuild\\Current\\Bin";
	std::string vcxprojDir = std::filesystem::current_path().string() + "\\..\\GEAR_NATIVE_SCRIPT\\";
	std::string solutionDir = std::filesystem::current_path().string() + "\\..\\";
	std::string _nativeScriptDir = std::filesystem::current_path().string() + "\\" + nativeScriptDir;

	if (!CheckPath(s_BuildScriptPath) && !CheckPath(msBuildDir) && !CheckPath(vcxprojDir) && !CheckPath(_nativeScriptDir))
		return;
	
	//Build and Link Dynamic Library
	#ifdef _DEBUG
	std::string command = "MSBuild " + vcxprojDir + "GEAR_NATIVE_SCRIPT.vcxproj -t:build -p:Platform=x64 -p:Configuration=Debug -p:SolutionDir=" + solutionDir + " -p:ApplicationNativeScriptsDir=" + _nativeScriptDir;
	#else
	std::string command = "MSBuild " + vcxprojDir + "GEAR_NATIVE_SCRIPT.vcxproj -t:build -p:Platform=x64 -p:Configuration=Release -p:SolutionDir=" + solutionDir + " -p:ApplicationNativeScriptsDir=" + _nativeScriptDir;
	#endif

	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj using MSBuild\n");
	GEAR_PRINTF(("Executing: " + msBuildDir + "> " + command + "\n").c_str());
	int errorCode = system(("cd " + msBuildDir + " && " + command).c_str());
	GEAR_PRINTF("'MSBuild.exe' has exited with code %d (0x%x).\n", errorCode, errorCode);
	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj finished.\n\n");

	if (!(errorCode == 0))
		system("pause");

	system("cls");
}

DynamicLibrary::LibraryHandle NativeScriptManager::Load()
{
	if (!CheckPath(s_BuildScriptPath))
		return DynamicLibrary::LibraryHandle(0);

	std::string dllFullpath = s_BuildScriptPath + "GEAR_NATIVE_SCRIPT.dll";
	return DynamicLibrary::Load(dllFullpath.c_str());
}

void NativeScriptManager::Unload(DynamicLibrary::LibraryHandle& libraryHandle)
{
	DynamicLibrary::Unload(libraryHandle);
	libraryHandle = 0;
}

INativeScript* NativeScriptManager::LoadScript(DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName)
{
	typedef INativeScript* (*PFN_LoadScript)();
	
	INativeScript* ns = nullptr;
	PFN_LoadScript LoadScript = (PFN_LoadScript)DynamicLibrary::LoadFunction(libraryHandle, "LoadScript_" + nativeScriptName);
	if(LoadScript)
		ns = (INativeScript*)LoadScript();
	
	return ns;
}

void NativeScriptManager::UnloadScript(DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, INativeScript*& nativeScript)
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
		GEAR_WARN(ErrorCode::SCENE | ErrorCode::INVALID_PATH, "%s does not exist.", s_BuildScriptPath.c_str());
		return false;
	}
	return true;
}
