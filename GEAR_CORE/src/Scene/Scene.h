#pragma once
#include "gear_core_common.h"
#include "entt.hpp"

#include "Components.h"


namespace gear
{
namespace core { class Timer; }
namespace graphics { class Renderer;}

namespace scene
{
	class Entity;
	class INativeScript;

	class Scene
	{
	public:
		struct CreateInfo
		{
			std::string debugName;
			std::string filepath;
			std::string nativeScriptDir;
		};
	
	public:
		CreateInfo m_CI;

	public:
		Scene(CreateInfo* pCreateInfo);
		~Scene();
	
		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		void OnUpdate(Ref<graphics::Renderer>& m_Renderer, core::Timer& timer);

		entt::registry& GetRegistry();

		void LoadNativeScriptLibrary();
		void UnloadNativeScriptLibrary();

		void LoadFromFile();
		void SaveToFile();
		inline void Play() { m_Playing = true; }
		inline void Stop() { m_Playing = false; }
	
	private:
		entt::registry m_Registry;
		bool m_Playing = false;

		friend class Entity;
	};
}
}
