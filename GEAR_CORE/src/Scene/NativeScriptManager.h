#pragma once
#include "ARC/src/DynamicLibrary.h"

namespace gear
{
	namespace scene
	{
		class INativeScript;

		class GEAR_API NativeScriptManager
		{
		public:
			static void Build(const std::string& nativeScriptDir);
			static arc::DynamicLibrary::LibraryHandle Load();
			static void Unload(arc::DynamicLibrary::LibraryHandle& libraryHandle);

			static INativeScript* LoadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName);
			static void UnloadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, INativeScript*& nativeScript);

		private:
			static std::filesystem::path GetMSBuildPath();
			static bool CheckPath(const std::filesystem::path& directory);
			static std::filesystem::file_time_type GetLibraryLastWriteTime();
			static std::filesystem::file_time_type GetSourceLastWriteTime(const std::filesystem::path& nativeScriptPath);
			static void CheckWin32BOOL(BOOL success);
			static void ForceUnloadPDB();

		private:
			static std::filesystem::path s_BuildScriptPath;
		};
	}
}