#pragma once

#include "ARC/src/DynamicLibrary.h"

namespace gear
{
namespace scene
{
	class INativeScript;
	
	typedef std::string ScriptingLibrary;

	class GEAR_API NativeScriptManager
	{
	public:
		static void Build(const std::string& nativeScriptDir);
		static arc::DynamicLibrary::LibraryHandle Load();
		static void Unload(arc::DynamicLibrary::LibraryHandle& libraryHandle);

		static INativeScript* LoadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName);
		static void UnloadScript(arc::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, INativeScript*& nativeScript);


	private:
		static bool CheckPath(const std::string& directory);

		static std::string s_BuildScriptPath;
	};
}
}