#include "gear_core_common.h"
#include "Scene/NativeScriptManager.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj_core.h>

#include "ARC/src/WindowsErrorHandling.h"

using namespace arc;
using namespace gear;
using namespace scene;

#ifdef _DEBUG
std::filesystem::path NativeScriptManager::s_BuildScriptPath = std::filesystem::path(BUILD_DIR) / "bin/Debug/";
static bool debug = true;
#else
std::filesystem::path NativeScriptManager::s_BuildScriptPath = std::filesystem::path(BUILD_DIR) / "bin/Release/";
static bool debug = false;
#endif

void NativeScriptManager::Build(const std::string& nativeScriptDir)
{
	std::filesystem::path msBuildPath = GetMSBuildPath();
	std::filesystem::path vcxprojPath = std::filesystem::path(BUILD_DIR) / "GEAR_NATIVE_SCRIPT/";
	std::filesystem::path solutionPath = std::filesystem::path(BUILD_DIR);
	std::filesystem::path sourcePath = std::filesystem::path(SOURCE_DIR);
	std::filesystem::path nativeScriptPath = nativeScriptDir;

	if (!CheckPath(s_BuildScriptPath) && !CheckPath(msBuildPath) && !CheckPath(vcxprojPath) && !CheckPath(solutionPath) && !CheckPath(nativeScriptPath))
		return;

	if (GetLibraryLastWriteTime() > GetSourceLastWriteTime(nativeScriptPath))
		return;

	//CMake Configure and Generate GEAR_NATIVE_SCRIPT/CMakeLists.txt
	{
		std::string command;
		command += "cmake.exe "; //Assume CMake is on the System Path.
		command += "-S " + arc::ToString(sourcePath.native()) + " ";
		command += "-B " + arc::ToString(solutionPath.native()) + " ";
		command += "-D APPLICATION_NATIVE_SCRIPTS_DIR=" + arc::ToString(nativeScriptPath.native());
		GEAR_INFO(true, "GEAR_CORE: Configure and Generate GEAR_NATIVE_SCRIPT.vcxproj from GEAR_NATIVE_SCRIPT/CMakeLists.txt using CMake.");
		GEAR_INFO(true, ("Executing: " + command).c_str());

		DWORD dwExitCode = CallProcessCommandLine(command);

		GEAR_INFO(true, "'cmake.exe' has exited with code %d (0x%x).", dwExitCode, dwExitCode);
		GEAR_INFO(true, "GEAR_CORE: Configure and Generate GEAR_NATIVE_SCRIPT.vcxproj from GEAR_NATIVE_SCRIPT/CMakeLists.txt finished.");

		if (dwExitCode != 0)
		{
			GEAR_WARN(true, "GEAR_CORE: Failed to Configure and Generate GEAR_NATIVE_SCRIPT.vcxproj.");
		}
	}

	//Build and Link Dynamic Library
	{
		std::string command;
		command += arc::ToString(msBuildPath.native()) + "MSBuild.exe ";
		command += arc::ToString(vcxprojPath.native()) + "GEAR_NATIVE_SCRIPT.vcxproj ";
		command += "-t:build ";
		//command += "-p:Platform=x64 ";
		command += debug ? "-p:Configuration=Debug " : "-p:Configuration=Release ";
		command += "-p:SolutionDir=" + arc::ToString(solutionPath.native()) + " ";
		command += "-p:BuildProjectReferences=false "; //Ignore Project's dependencies. GEAR_CORE is already loaded.

		GEAR_INFO(true, "GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj using MSBuild.");
		GEAR_INFO(true, ("Executing: " + command).c_str());

		DWORD dwExitCode = CallProcessCommandLine(command);

		std::filesystem::remove_all(sourcePath / "buildx64"); //CMake is being dumb and adding this redundant folder.

		GEAR_INFO(true, "'MSBuild.exe' has exited with code %d (0x%x).", dwExitCode, dwExitCode);
		GEAR_INFO(true, "GEAR_CORE: Build GEAR_NATIVE_SCRIPT.dll from GEAR_NATIVE_SCRIPT.vcxproj finished.");

		if (dwExitCode != 0)
		{
			GEAR_WARN(true, "GEAR_CORE: Failed to Build GEAR_NATIVE_SCRIPT.dll.");
		}
	}

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
	//ForceUnloadPDB();

	DynamicLibrary::Unload(libraryHandle);
	libraryHandle = 0;
}

NativeScript* NativeScriptManager::LoadScript(DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName)
{
	typedef NativeScript* (*PFN_LoadScript)();
	
	NativeScript* nativeScript = nullptr;
	PFN_LoadScript LoadScript = (PFN_LoadScript)DynamicLibrary::LoadFunction(libraryHandle, "LoadScript_" + nativeScriptName);
	if(LoadScript)
		nativeScript = (NativeScript*)LoadScript();
	
	return nativeScript;
}

void NativeScriptManager::UnloadScript(DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, NativeScript*& nativeScript)
{
	typedef void (*PFN_UnloadScript)(NativeScript*);

	PFN_UnloadScript UnloadScript = (PFN_UnloadScript)DynamicLibrary::LoadFunction(libraryHandle, "UnloadScript_" + nativeScriptName);
	if (UnloadScript)
	{
		UnloadScript(nativeScript);
		nativeScript = nullptr;
	}
}

uint32_t NativeScriptManager::CallProcessCommandLine(const std::string& commandLine)
{
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi = { 0 };
	DWORD process = CreateProcessA(NULL, (LPSTR)commandLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	CheckWin32BOOL(process);

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return dwExitCode;
}

std::filesystem::path NativeScriptManager::GetMSBuildPath()
{
	//C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Msbuild\\Current\\Bin\\MSBuild.exe
	
	std::filesystem::path vswherePath = "";
	std::filesystem::path vsInstallaionPath = "";

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
	std::filesystem::path msBuildPath = vsInstallaionPath.native() + L"\\Msbuild\\Current\\Bin\\";

	return msBuildPath;
}

bool NativeScriptManager::CheckPath(const std::filesystem::path& directory)
{
	if (!std::filesystem::exists(directory))
	{
		GEAR_WARN(ErrorCode::SCENE | ErrorCode::INVALID_PATH, "%s does not exist.", directory.string().c_str());
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

void NativeScriptManager::CheckWin32BOOL(bool success)
{
	if (!success)
	{
		GEAR_FATAL(ErrorCode::SCENE | ErrorCode::FUNC_FAILED, "Call to Win32 API failed with error: %s", arc::GetLastErrorToString(GetLastError()));
	}
}

#include <RestartManager.h>
#pragma comment(lib, "Rstrtmgr.lib")

#include <Winternl.h>
#pragma warning(push)
#pragma warning(disable : 4005)
#include <ntstatus.h>
#pragma warning(pop)
#include <Psapi.h>
#pragma comment(lib, "Ntdll.lib")

typedef struct _SYSTEM_HANDLE {
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

void NativeScriptManager::ForceUnloadPDB()
{
	//https://blog-molecular--matters-com.cdn.ampproject.org/v/s/blog.molecular-matters.com/2017/05/09/deleting-pdb-files-locked-by-visual-studio/amp/?amp_gsa=1&amp_js_v=a9&usqp=mq331AQKKAFQArABIIACAw%3D%3D#amp_tf=From%20%251%24s&aoh=16578355814075&referrer=https%3A%2F%2Fwww.google.com&ampshare=https%3A%2F%2Fblog.molecular-matters.com%2F2017%2F05%2F09%2Fdeleting-pdb-files-locked-by-visual-studio%2F
	//https://devblogs.microsoft.com/oldnewthing/20120217-00/?p=8283
	//http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FNT%20Objects%2FFile%2FNtDeleteFile.html

	DWORD session;
	WCHAR sessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
	DWORD error = RmStartSession(&session, 0, sessionKey);
	if (error == ERROR_SUCCESS)
	{
		std::filesystem::path path = s_BuildScriptPath / "GEAR_NATIVE_SCRIPT.pdb";
		const wchar_t* cwstr_path = path.native().c_str();
		error = RmRegisterResources(session, 1, &cwstr_path, 0, NULL, 0, NULL);
		if (error == ERROR_SUCCESS)
		{
			DWORD reason;
			UINT procInfoNeeded;
			UINT procInfoCount = 1;
			std::vector<RM_PROCESS_INFO> procInfos(procInfoCount);

			error = RmGetList(session, &procInfoNeeded, &procInfoCount, procInfos.data(), &reason);
			procInfoCount = procInfoNeeded;
			procInfos.resize(procInfoCount);
			error = RmGetList(session, &procInfoNeeded, &procInfoCount, procInfos.data(), &reason);

			if (error == ERROR_SUCCESS)
			{
				int SystemHandleInformation = 16;
				PSYSTEM_HANDLE_INFORMATION pshi = nullptr;
				SYSTEM_HANDLE_INFORMATION shi = {0};
				NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemHandleInformation, &shi, sizeof(SYSTEM_HANDLE_INFORMATION), NULL);
				ULONG shiSize = (shi.HandleCount + 10000) * sizeof(SYSTEM_HANDLE) + sizeof(ULONG);

				pshi = (PSYSTEM_HANDLE_INFORMATION)malloc(shiSize);
				status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemHandleInformation, pshi, shiSize, NULL);
				if (status == 0)
				{
					std::vector<SYSTEM_HANDLE> systemHandles;
					for (ULONG i = 0; i < pshi->HandleCount; i++)
					{
						const BYTE ObjectTypeFile = 0x27;
						SYSTEM_HANDLE& sh = pshi->Handles[i];
						if (sh.ObjectTypeNumber != ObjectTypeFile)
							continue;

						for (const RM_PROCESS_INFO& procInfo : procInfos)
						{
							if (sh.ProcessId == (ULONG)procInfo.Process.dwProcessId && sh.GrantedAccess != 0x00120189)
							{
								HANDLE process = OpenProcess(PROCESS_DUP_HANDLE, FALSE, sh.ProcessId);
								HANDLE dupHandle = 0;
								BOOL success = DuplicateHandle(process, (HANDLE)sh.Handle, GetCurrentProcess(), &dupHandle, 0, 0, 0);
								if (success)
								{
									int ObjectNameInformation = 1;
									PVOID objectNameInfo = malloc(0x1000);
									ULONG returnLength = 0;
									status = NtQueryObject(dupHandle, (OBJECT_INFORMATION_CLASS)ObjectNameInformation, objectNameInfo, 0x1000, &returnLength);
									if (status == 0)
									{
										UNICODE_STRING objectName = *(PUNICODE_STRING)objectNameInfo;
										std::wstring objectName_wstr = objectName.Length ? std::wstring(((PUNICODE_STRING)objectNameInfo)->Buffer) : L"";
										if (objectName_wstr.find(L"GEAR_NATIVE_SCRIPT.pdb") != std::wstring::npos)
										{
											systemHandles.push_back(sh);
										}
									}
									free(objectNameInfo);
								}
								CloseHandle(dupHandle);
								CloseHandle(process);
							}
						}
					}

					for (const SYSTEM_HANDLE& sh : systemHandles)
					{
						HANDLE process = OpenProcess(PROCESS_DUP_HANDLE, FALSE, sh.ProcessId);
						status = DuplicateHandle(process, (HANDLE)sh.Handle, 0, NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
						CloseHandle((HANDLE)sh.Handle);
						CloseHandle(process);
					}
					HANDLE file = CreateFileW(path.native().c_str(),
						(GENERIC_READ | GENERIC_WRITE),
						FILE_SHARE_DELETE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_DELETE_ON_CLOSE,
						NULL);
					CloseHandle(file);
				}
				free(pshi);
			}
		}
	}
	RmEndSession(session);

}
