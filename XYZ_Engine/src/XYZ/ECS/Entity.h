#pragma once

#include "ECSManager.h"


namespace XYZ {

	class Scene;
	class Entity
	{
	public:
		Entity()
			:
			m_ID(ECSManager::CreateEntity()),
			m_Scene(nullptr)
		{
		}

		Entity(Scene* scene)
			: 
			m_ID(ECSManager::CreateEntity()),
			m_Scene(scene)
		{
		}

		Entity(const Entity& other)
			: 
			m_ID(other.m_ID),
			m_Scene(other.m_Scene)
		{}

		template<typename T>
		T* GetComponent()
		{
			return ECSManager::GetComponent<T>(m_ID);
		}

		template <typename T>
		const T* GetComponent() const
		{
			return ECSManager::GetComponent<T>(m_ID);
		}

		template <typename T>
		T* AddComponent(const T& component)
		{
			auto addedComponent = ECSManager::AddComponent<T>(m_ID, component);
			if (m_Scene)
				m_Scene->onEntityModified<T>(addedComponent,*this);
			return addedComponent;
		}

		template <typename T>
		bool HasComponent()
		{
			return ECSManager::Contains<T>(m_ID);
		}

		void Destroy()
		{
			ECSManager::DestroyEntity(m_ID);
		}

		Entity& operator =(const Entity& other)
		{
			m_ID = other.m_ID;
			m_Scene = other.m_Scene;
			return *this;
		}

		operator bool () const
		{
			return m_Scene;
		}

		operator uint32_t () const { return m_ID; }

	private:
		uint32_t m_ID;
		Scene* m_Scene;


		friend class Scene;
	};
}