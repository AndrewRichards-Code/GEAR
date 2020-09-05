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
		};
	
	public:
		CreateInfo m_CI;

	public:
		Scene(CreateInfo* pCreateInfo);
		~Scene();
	
		Entity CreateEntity();
		void* Get(uint32_t id);
		void OnUpdate(gear::Ref<graphics::Renderer>& renderer, core::Timer& timer);

		void LoadNativeScriptLibrary();
		void UnloadNativeScriptLibrary();

		void LoadFromFile();
		void SaveToFile();
		void TogglePlay() { m_Playing = !m_Playing; std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
	
	private:
		entt::registry m_Registry;
		bool m_Playing = false;

		friend class Entity;
	};
}
}
