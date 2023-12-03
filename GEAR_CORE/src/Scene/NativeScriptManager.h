#pragma once
#include "ARC/src/DynamicLibrary.h"

namespace gear
{
	namespace scene
	{
		class NativeScript;

		class GEAR_API NativeScriptManager
		{
		public:
			static void Build(const std::string& nativeScriptDir);
			static arc::DynamicLibrary::LibraryHandle Load();
			static void Unload(arc::DynamicLibrary::LibraryHandle& libraryHandle);

			static NativeScript* LoadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName);
			static void UnloadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, NativeScript*& nativeScript);

		private:
			static uint32_t CallProcessCommandLine(const std::string& commandLine);
			static std::filesystem::path GetMSBuildPath();
			static bool CheckPath(const std::filesystem::path& directory);
			static std::filesystem::file_time_type GetLibraryLastWriteTime();
			static std::filesystem::file_time_type GetSourceLastWriteTime(const std::filesystem::path& nativeScriptPath);
			static void CheckWin32BOOL(bool success);
			static void ForceUnloadPDB();

		private:
			static std::filesystem::path s_BuildScriptPath;
		};
	}
}