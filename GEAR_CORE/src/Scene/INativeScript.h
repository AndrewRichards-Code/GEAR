#pragma once

#if defined(_MSC_VER)
#pragma warning(disable : 4251) //Disables 'Needs to have dll-interface'  warning C4251
#endif

#ifdef _WIN64
#if defined(GEAR_SCRIPT_API_IMPORT)
#define GEAR_SCRIPT_API __declspec(dllimport)
#else
#define GEAR_SCRIPT_API __declspec(dllexport)
#endif
#else
#define GEAR_SCRIPT_API
#endif

#include "Scene/Entity.h"

namespace gear
{
	namespace scene
	{
		class GEAR_SCRIPT_API INativeScript
		{
		public:
			INativeScript() = default;
			virtual ~INativeScript() = default;

			virtual void OnCreate() {}
			virtual void OnDestroy() {}
			virtual void OnUpdate(float deltaTime) {}

			Entity& GetEntity() { return m_Entity; }

		private:
			//This function can not be called by a GEAR_NATIVE_SCRIPT.dll
			void SetEntity(const Entity& entity) { m_Entity = entity; }

		private:
			Entity m_Entity;

			friend class gear::scene::Scene;
		};
	}
}

#define GEAR_LOAD_SCRIPT(T) extern "C" GEAR_SCRIPT_API INativeScript* LoadScript_##T() { return (INativeScript*)(new T()); }
#define GEAR_UNLOAD_SCRIPT(T) extern "C" GEAR_SCRIPT_API void UnloadScript_##T(INativeScript* nativeScript) { if(nativeScript) { delete (T*)nativeScript; nativeScript = nullptr; } }