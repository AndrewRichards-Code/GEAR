#pragma once
#include "gear_core_common.h"
#include "entt.hpp"

#include "Components.h"

namespace gear
{
namespace core { class Timer; }
namespace graphics { class Renderer; class Window; }

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
		inline void ClearEntities() { m_Registry.clear(); }

		void LoadNativeScriptLibrary();
		void UnloadNativeScriptLibrary();

		void LoadEntity(nlohmann::json& references, nlohmann::json& entity, Entity entityID, const Ref<graphics::Window>& window);
		void LoadFromFile(const std::string& filepath, const Ref<graphics::Window>& window);
		
		void SaveEntity(nlohmann::json& references, nlohmann::json& entity, Entity entityID);
		void SaveToFile(const std::string& filepath);

		inline void Play() { m_Playing = true; }
		inline void Stop() { m_Playing = false; }
		inline const std::string& GetFilepath() const { return m_Filepath; }
	
	private:
		core::UUID m_UUID;
		entt::registry m_Registry;
		bool m_Playing = false;
		std::string m_Filepath;

	public:
		std::map<core::UUID, Ref<objects::Mesh>> s_Meshes;
		std::map<core::UUID, Ref<objects::Material>> s_Materials;
		std::map<core::UUID, Ref<objects::FontLibrary::Font>> s_Fonts;

		friend class Entity;
	};
}
}
