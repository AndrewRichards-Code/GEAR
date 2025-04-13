#pragma once
#include "gear_core_common.h"
#include "Core/UUID.h"
#include "Asset/Asset.h"

#define ENTT_CORE_ENUM_HPP
#include "ENTT/single_include/entt/entt.hpp"
#include "ARC/External/json/json.hpp"

namespace gear
{
	namespace core
	{
		class Timer;
	}
	namespace graphics::rendering
	{
		class Renderer;
	}
	namespace scripting
	{
		class NativeScript;
	}
	namespace scene
	{
		class Entity;

		struct UUIDComponent;
		struct NameComponent;
		struct TransformComponent;
		struct CameraComponent;
		struct LightComponent;
		struct ModelComponent;
		struct SkyboxComponent;
		struct TextComponent;
		struct NativeScriptComponent;

		class GEAR_SCENE_API Scene : public asset::Asset
		{
		public:
			struct CreateInfo
			{
				std::string debugName;
			};
			enum class State : uint32_t
			{
				STOP,
				EDIT = STOP,
				PLAY,
			};

		public:
			CreateInfo m_CI;

		public:
			Scene(CreateInfo* pCreateInfo);
			~Scene();

			Entity CreateEntity();
			void DestroyEntity(Entity entity);

			void OnUpdate(Ref<graphics::rendering::Renderer> m_Renderer, core::Timer& timer);

			entt::registry& GetRegistry();
			inline void ClearEntities() { m_Registry.clear(); }

			inline const State& GetState() const { return m_State; }
			inline void Play() { m_State = State::PLAY; }
			inline void Stop() { m_State = State::STOP; }

			inline static arc::DynamicLibrary::LibraryHandle& GetNativeScriptLibrary() { return s_NativeScriptLibrary; }

		private:
			void LoadNativeScripts();
			void UnloadNativeScripts();

		private:
			core::UUID m_UUID;
			entt::registry m_Registry;
			State m_State = State::STOP;

			static arc::DynamicLibrary::LibraryHandle s_NativeScriptLibrary;

			friend class Entity;
		};
	}
}
