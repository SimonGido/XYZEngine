#pragma once

#include "ECSManager.h"
#include "XYZ/Scene/Scene.h"

namespace XYZ {

	class Entity
	{
	public:
		Entity()
			:
			m_ID(NULL_ENTITY),
			m_Scene(nullptr)
		{
		}

		Entity(Scene* scene)
			: 
			m_Scene(scene),
			m_ID(m_Scene->m_ECS->CreateEntity())
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
			return m_Scene->m_ECS->GetComponent<T>(m_ID);
		}

		template <typename T>
		const T* GetComponent() const
		{
			return m_Scene->m_ECS->GetComponent<T>(m_ID);
		}

		template <typename T>
		T* AddComponent(const T& component)
		{
			auto addedComponent = m_Scene->m_ECS->AddComponent<T>(m_ID, component);
			if (m_Scene)
				m_Scene->onEntityModified<T>(addedComponent,*this);
			return addedComponent;
		}

		template <typename T>
		bool HasComponent()
		{
			return m_Scene->m_ECS->Contains<T>(m_ID);
		}

		void Destroy()
		{
			m_Scene->m_ECS->DestroyEntity(m_ID);
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
		Scene* m_Scene;
		uint32_t m_ID;


		friend class Scene;
	};
}