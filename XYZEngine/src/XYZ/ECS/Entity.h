#pragma once

#include "ECSManager.h"

namespace XYZ {
	class Entity
	{
	public:
		Entity()
			:
			m_ECS(nullptr),
			m_ID(NULL_ENTITY)
		{
		}

		Entity(ECSManager* ecs)
			:
			m_ECS(ecs),
			m_ID(m_ECS->CreateEntity())
		{

		}

		Entity(const Entity& other)
			:
			m_ECS(other.m_ECS),
			m_ID(other.m_ID)
		{}

		Entity(uint32_t id, ECSManager* ecs)
			:
			m_ECS(ecs),
			m_ID(id)
		{

		}

		template<typename T>
		T& GetGroupComponent()
		{
			return m_ECS->GetGroupComponent<T>(m_ID);
		}

		template<typename T>
		const T& GetGroupComponent() const
		{
			return m_ECS->GetGroupComponent<T>(m_ID);
		}

		
		template<typename T>
		T& GetStorageComponent()
		{
			return m_ECS->GetStorageComponent<T>(m_ID);
		}

		template<typename T>
		const T& GetStorageComponent() const
		{
			return m_ECS->GetStorageComponent<T>(m_ID);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_ECS->GetComponent<T>(m_ID);
		}
		
		template <typename T>
		const T& GetComponent() const
		{
			return m_ECS->GetComponent<T>(m_ID);
		}
		
		template <typename T>
		T& AddComponent(const T& component)
		{
			return m_ECS->AddComponent<T>(m_ID, component);
		}
		template <typename T>
		void RemoveComponent()
		{
			m_ECS->RemoveComponent<T>(m_ID);
		}
	
		template <typename T>
		bool HasComponent() const
		{
			return m_ECS->Contains<T>(m_ID);
		}
		
		void Destroy()
		{
			m_ECS->DestroyEntity(m_ID);
		}
		
		Entity& operator =(const Entity& other)
		{
			m_ECS = other.m_ECS;
			m_ID = other.m_ID;
			return *this;
		}
		
		bool operator ==(const Entity& other) const
		{
			return (m_ID == other.m_ID && m_ECS == other.m_ECS);
		}
		
		operator bool() const
		{
			return m_ECS && m_ID != NULL_ENTITY;
		}

		operator uint32_t () const { return m_ID; }

	private:
		ECSManager* m_ECS;
		uint32_t    m_ID;


		friend class Scene;
	};
}