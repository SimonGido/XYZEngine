#pragma once

#include "Scene.h"


namespace XYZ {
	class Entity
	{
	public:
		Entity()
			:
			m_Scene(nullptr),
			m_ID(NULL_ENTITY)
		{
		}

		Entity(Scene* scene)
			:
			m_Scene(scene),
			m_ID(m_Scene->m_ECS.CreateEntity())
		{

		}

		Entity(const Entity& other)
			:
			m_Scene(other.m_Scene),
			m_ID(other.m_ID)
		{}

		Entity(uint32_t id, Scene* scene)
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
		
		template <typename T>
		T& AddComponent(const T& component)
		{
			return m_Scene->m_ECS.AddComponent<T>(m_ID, component);
		}
		
	
		template <typename T>
		bool HasComponent() const
		{
			return m_Scene->m_ECS.Contains<T>(m_ID);
		}
		
		void Destroy()
		{
			m_Scene->m_ECS.DestroyEntity(m_ID);
		}
		
		Entity& operator =(const Entity& other)
		{
			m_Scene = other.m_Scene;
			m_ID = other.m_ID;
			return *this;
		}
		
		bool operator ==(const Entity& other) const
		{
			return (m_ID == other.m_ID && m_Scene == other.m_Scene);
		}
		
		operator bool() const
		{
			return m_Scene && m_ID != NULL_ENTITY;
		}

		operator uint32_t () const { return m_ID; }

	private:
		Scene* m_Scene;
		uint32_t m_ID;


		friend class Scene;
	};
}