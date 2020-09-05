#pragma once

#include "Core/DynamicLibrary.h"

namespace gear
{
namespace scene
{
	class INativeScript;
	
	typedef std::string ScriptingLibrary;

	class NativeScriptManager
	{
	public:
		static void Build();
		static core::DynamicLibrary::LibraryHandle Load();
		static void Unload(core::DynamicLibrary::LibraryHandle& libraryHandle);

		static INativeScript* LoadScript(core::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName);
		static void UnloadScript(core::DynamicLibrary::LibraryHandle& libraryHandle, const std::string& nativeScriptName, INativeScript*& nativeScript);


	private:
		static bool CheckPath(const std::string& directory);

		static std::string s_BuildScriptPath;
	};
}
}