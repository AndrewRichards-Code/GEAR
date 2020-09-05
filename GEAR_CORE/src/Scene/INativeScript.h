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

		CameraComponent*& GetCameraComponent() { return m_CameraComponent; }
		LightComponent*& GetLightComponent() { return m_LightComponent; }
		ModelComponent*& GetModelComponent() { return m_ModelComponent;  }
	
	private:
		//This function can not be called by a GEAR_NATIVE_SCRIPT.dll
		void SetComponents(NativeScriptComponent& nsc)
		{
			if (nsc.entity->HasComponent<CameraComponent>())
				m_CameraComponent = &(nsc.entity->GetComponent<CameraComponent>());
			if (nsc.entity->HasComponent<LightComponent>())
				m_LightComponent = &(nsc.entity->GetComponent<LightComponent>());
			if (nsc.entity->HasComponent<ModelComponent>())
				m_ModelComponent = &(nsc.entity->GetComponent<ModelComponent>());
		};

	private:
		CameraComponent* m_CameraComponent = nullptr;
		LightComponent* m_LightComponent = nullptr;
		ModelComponent* m_ModelComponent = nullptr;
		
		friend class gear::scene::Scene;
	};
}
}

#define GEAR_LOAD_SCRIPT(T) extern "C" GEAR_SCRIPT_API INativeScript* LoadScript_##T() { return (INativeScript*)(new T()); }
#define GEAR_UNLOAD_SCRIPT(T) extern "C" GEAR_SCRIPT_API void UnloadScript_##T(INativeScript* nativeScript) { if(nativeScript) { delete (T*)nativeScript; nativeScript = nullptr; } }