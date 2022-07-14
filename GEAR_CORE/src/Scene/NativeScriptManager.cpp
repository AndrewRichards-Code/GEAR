#include "gear_core_common.h"
#include "Scene/NativeScriptManager.h"

using namespace arc;
using namespace gear;
using namespace scene;

#ifdef _DEBUG
std::filesystem::path NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path() / "..\\bin\\x64\\Debug\\";
static bool debug = true;
#else
std::filesystem::path NativeScriptManager::s_BuildScriptPath = std::filesystem::current_path() + "..\\bin\\x64\\Release\\";
static bool debug = false;
#endif

void NativeScriptManager::Build(const std::string& nativeScriptDir)
{
	std::filesystem::path msBuildPath = GetMSBuildPath();
	std::filesystem::path vcxprojPath = std::filesystem::current_path() / "..\\GEAR_NATIVE_SCRIPT\\";
	std::filesystem::path solutionPath = std::filesystem::current_path() / "..\\";
	std::filesystem::path nativeScriptPath = std::filesystem::current_path() / nativeScriptDir;

	if (!CheckPath(s_BuildScriptPath) && !CheckPath(msBuildPath) && !CheckPath(vcxprojPath) && !CheckPath(solutionPath) && !CheckPath(nativeScriptPath))
		return;

	if (GetLibraryLastWriteTime() > GetSourceLastWriteTime(nativeScriptPath))
		return;

	//Build and Link Dynamic Library
	std::string command;
	command += arc::ToString(msBuildPath.native()) + "MSBuild.exe ";
	command += arc::ToString(vcxprojPath.native()) + "GEAR_NATIVE_SCRIPT.vcxproj ";
	command += "-t:build ";
	command += "-p:Platform=x64 ";
	command += debug ? "-p:Configuration=Debug " : "-p:Configuration=Release ";
	command += "-p:SolutionDir=" + arc::ToString(solutionPath.native()) + " ";
	command += "-p:ApplicationNativeScriptsDir=" + arc::ToString(nativeScriptPath.native());


	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj using MSBuild\n");
	GEAR_PRINTF(("Executing: " + command + "\n").c_str());

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi = { 0 };
	BOOL process = CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	CheckWin32BOOL(process);

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	GEAR_PRINTF("'MSBuild.exe' has exited with code %d (0x%x).\n", dwExitCode, dwExitCode);
	GEAR_PRINTF("GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj finished.\n\n");

	if (!(dwExitCode == 0))
		system("pause");

	system("cls");
}

DynamicLibrary::LibraryHandle NativeScriptManager::Load()
{
	if (!CheckPath(s_BuildScriptPath))
		return DynamicLibrary::LibraryHandle(0);

	std::string dllFullpath = s_BuildScriptPath.generic_string() + "GEAR_NATIVE_SCRIPT.dll";
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

std::filesystem::path NativeScriptManager::GetMSBuildPath()
{
	//C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Msbuild\\Current\\Bin\\MSBuild.exe
	
	std::filesystem::path vswherePath;
	std::filesystem::path vsInstallaionPath;
	std::filesystem::path msBuildPath;
	LPWSTR programFilesPath;
	SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, KF_FLAG_DEFAULT, 0, &programFilesPath);
	vswherePath = arc::ToString(programFilesPath);
	CoTaskMemFree(programFilesPath);
	vswherePath /= "Microsoft Visual Studio\\Installer\\";
	std::string cmd = arc::ToString(vswherePath.native() + L"vswhere.exe -latest -property \"installationPath\"");

	HANDLE hPipeRead, hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
	saAttr.lpSecurityDescriptor = NULL;
	BOOL pipe = CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0);
	CheckWin32BOOL(pipe);

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;
	si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi = { 0 };
	BOOL process = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	CheckWin32BOOL(process);
	WaitForSingleObject(pi.hProcess, 50);

	std::string buffer;
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	while (true)
	{
		BOOL success = PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL);
		if (!success || dwAvail == 0)
			break;
		buffer.resize(std::max(buffer.size(), static_cast<size_t>(dwAvail)));
		success = ReadFile(hPipeRead, buffer.data(), static_cast<DWORD>(buffer.size()), &dwRead, nullptr);
		if (!success || dwRead == 0)
			break;
	}

	CloseHandle(hPipeWrite);
	CloseHandle(hPipeRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::erase_if(buffer, [](char c) { return (c == '\n' || c == '\r'); });
	vsInstallaionPath = buffer;
	msBuildPath = vsInstallaionPath.native() + L"\\Msbuild\\Current\\Bin\\";

	return msBuildPath;
}

bool NativeScriptManager::CheckPath(const std::filesystem::path& directory)
{
	if (!std::filesystem::exists(directory))
	{
		GEAR_WARN(ErrorCode::SCENE | ErrorCode::INVALID_PATH, "%s does not exist.", directory.c_str());
		return false;
	}
	return true;
}

std::filesystem::file_time_type NativeScriptManager::GetLibraryLastWriteTime()
{
	std::filesystem::file_time_type time;
	std::filesystem::path path = s_BuildScriptPath / "GEAR_NATIVE_SCRIPT.dll";
	if (CheckPath(path))
		time = std::filesystem::last_write_time(path);
	
	return time;
}

std::filesystem::file_time_type NativeScriptManager::GetSourceLastWriteTime(const std::filesystem::path& nativeScriptPath)
{
	std::filesystem::file_time_type time;
	for (const std::filesystem::directory_entry& directory : std::filesystem::directory_iterator(nativeScriptPath))
	{
		if (directory.exists() 
			&& directory.is_regular_file() 
			&& directory.path().extension().compare(std::filesystem::path(".cpp")) == 0)
		{
			time = std::max(time, std::filesystem::last_write_time(directory));
		}
	}
	return time;
}

void NativeScriptManager::CheckWin32BOOL(BOOL success)
{
	if (!success)
	{
		GEAR_ASSERT(ErrorCode::SCENE | ErrorCode::FUNC_FAILED, "Call to Win32 API failed with error: %s", arc::GetLastErrorToString(GetLastError()));
	}
}
