#pragma once
#include "Scene.h"

namespace gear
{
namespace scene
{
	class Entity
	{
	public:
		struct CreateInfo
		{
			Scene* pScene;
		};

	public:
		CreateInfo m_CI;
	
	public:
		Entity() = default;
		Entity(CreateInfo* pCreateInfo);
		~Entity();

		template<typename T>
		bool HasComponent()
		{
			return m_CI.pScene->m_Registry.has<T>(m_Entity);
		}
		
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT,
					"Entity(0x%x) already has a %s.", m_Entity, typeid(T).name());
			}
			T& component = m_CI.pScene->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);

			//Addition per Type actions
			if (typeid(T) == typeid(NativeScriptComponent))
			{
				auto& nsc = GetComponent<NativeScriptComponent>();
				nsc.entity = this;
			}
			if (typeid(T) == typeid(CameraComponent))
			{
				GetComponent<NameComponent>() = GetComponent<CameraComponent>().GetCreateInfo().debugName;
				GetComponent<TransformComponent>() = GetComponent<CameraComponent>().GetCreateInfo().transform;
			}
			if (typeid(T) == typeid(LightComponent))
			{
				GetComponent<NameComponent>() = GetComponent<LightComponent>().GetCreateInfo().debugName;
				GetComponent<TransformComponent>() = GetComponent<LightComponent>().GetCreateInfo().transform;
			}
			if (typeid(T) == typeid(ModelComponent))
			{
				GetComponent<NameComponent>() = GetComponent<ModelComponent>().GetCreateInfo().debugName;
				GetComponent<TransformComponent>() = GetComponent<ModelComponent>().GetCreateInfo().transform;
			}

			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			if (!HasComponent<T>())
			{
				GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT,
					"Entity(0x%x) does not have a %s.", m_Entity, typeid(T).name());
			}
			return m_CI.pScene->m_Registry.get<T>(m_Entity);
		}

		template<typename T>
		T& RemoveComponent()
		{
			if (!HasComponent<T>())
			{
				GEAR_ASSERT(/*Level::ERROR,*/ ErrorCode::SCENE | ErrorCode::INVALID_COMPONENT,
					"Entity(0x%x) does not have a %s.", m_Entity, typeid(T).name());
			}
			return m_CI.pScene->m_Registry.remove<T>(m_Entity);
		}

		bool operator==(const Entity& other) const
		{
			return (m_Entity == other.m_Entity) && m_CI.pScene == other.m_CI.pScene;
		}
		bool operator!=(const Entity& other)
		{
			return !(*this == other);
		}
		
	//private:
		entt::entity m_Entity;
	};
}
}