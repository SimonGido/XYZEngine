#pragma once

#include "Scene.h"

namespace XYZ {
	class SceneEntity
	{
	public:
		SceneEntity()
			:
			m_Scene(nullptr)
		{
		}

		SceneEntity(const SceneEntity& other)
			:
			m_Scene(other.m_Scene),
			m_ID(other.m_ID)
		{}

		SceneEntity(Entity id, Scene* scene)
			:
			m_Scene(scene),
			m_ID(id)
		{
		}
	
		template<typename T>
		T& GetComponent()
		{
			return m_Scene->m_ECS.GetComponent<T>(m_ID);
		}

		template <typename T>
		const T& GetComponent() const
		{
			return m_Scene->m_ECS.GetComponent<T>(m_ID);
		}

		template <typename T, typename ...Args>
		T& EmplaceComponent(Args&&... args)
		{
			return m_Scene->m_ECS.EmplaceComponent<T, Args...>(m_ID, std::forward<Args>(args)...);
		}

		template <typename T>
		T& AddComponent(const T& component)
		{
			return m_Scene->m_ECS.AddComponent<T>(m_ID, component);
		}
		template <typename T>
		void RemoveComponent()
		{
			m_Scene->m_ECS.RemoveComponent<T>(m_ID);
		}

		template <typename T>
		bool HasComponent() const
		{
			return m_Scene->m_ECS.Contains<T>(m_ID);
		}

		void Destroy()
		{
			m_Scene->DestroyEntity(*this);
		}

		bool IsValid() const
		{
			return m_Scene && m_ID && m_Scene->m_ECS.IsValid(m_ID);
		}


		SceneEntity& operator =(const SceneEntity& other)
		{
			m_Scene = other.m_Scene;
			m_ID = other.m_ID;
			return *this;
		}

		bool operator ==(const SceneEntity& other) const
		{
			return (m_ID == other.m_ID && m_Scene == other.m_Scene);
		}

		
		operator uint32_t () const { return m_ID; }

	private:
		Scene*   m_Scene;
		Entity   m_ID;


		friend class Scene;
		friend class SceneSerializer;
		friend class ScriptEngine;
	};
}