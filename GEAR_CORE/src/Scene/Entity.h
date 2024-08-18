#pragma once
#include "Scene/Scene.h"
#include "Scene/Components.h"

namespace gear
{
	namespace scene
	{
		class GEAR_API Entity
		{
		public:
			struct CreateInfo
			{
				Scene* pScene;
			};

		public:
			CreateInfo m_CI;
			entt::entity m_Entity = entt::entity(~0);

		public:
			Entity() = default;
			Entity(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;
				m_Entity = m_CI.pScene->m_Registry.create();
			}
			~Entity() = default;

			template<typename T>
			bool HasComponent() const
			{
				return m_CI.pScene->m_Registry.all_of<T>(m_Entity);
			}

			template<typename T, typename... Args>
			T& AddComponent(Args&&... args)
			{
				if (HasComponent<T>())
				{
					GEAR_FATAL(ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT, "Entity(0x%x) already has a %s.", m_Entity, typeid(T).name());
				}
				T& component = m_CI.pScene->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);

				//Additional per Type actions
				if (typeid(T) == typeid(NativeScriptComponent))
				{
					GetComponent<NativeScriptComponent>().entity = this;
				}

				return component;
			}

			template<typename T>
			T& GetComponent() const
			{
				if (!HasComponent<T>())
				{
					GEAR_FATAL(ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT, "Entity(0x%x) does not have a %s.", m_Entity, typeid(T).name());
				}
				return m_CI.pScene->m_Registry.get<T>(m_Entity);
			}

			template<typename T>
			void RemoveComponent()
			{
				if (!HasComponent<T>())
				{
					GEAR_FATAL(ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT, "Entity(0x%x) does not have a %s.", m_Entity, typeid(T).name());
				}
				m_CI.pScene->m_Registry.remove<T>(m_Entity);
			}

			bool operator==(const Entity& other) const
			{
				return (m_Entity == other.m_Entity) && m_CI.pScene == other.m_CI.pScene;
			}
			bool operator!=(const Entity& other)
			{
				return !(*this == other);
			}
			operator bool()
			{
				return static_cast<uint32_t>(m_Entity) != ~0;
			}
			const core::UUID& GetUUID() const
			{
				return GetComponent<UUIDComponent>().uuid;
			}
		};
	}
}