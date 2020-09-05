#pragma once

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
			m_ID(m_Scene->m_ECS.CreateEntity())
		{
			
		}

		Entity(const Entity& other)
			: 
			m_ID(other.m_ID),
			m_Scene(other.m_Scene)
		{}

		Entity(uint32_t id, Scene* scene)
			:
			m_ID(id), 
			m_Scene(scene)
		{

		}

		template<typename T>
		T* GetComponent()
		{
			return m_Scene->m_ECS.GetComponent<T>(m_ID);
		}

		template <typename T>
		const T* GetComponent() const
		{
			return m_Scene->m_ECS.GetComponent<T>(m_ID);
		}

		template <typename T>
		T* AddComponent(const T& component)
		{
			return m_Scene->m_ECS.AddComponent<T>(m_ID, component);
		}
		
		template <typename T, typename ...Args>
		T* EmplaceComponent(Args&&...args)
		{
			return m_Scene->m_ECS.EmplaceComponent<T>(m_ID, std::forward<Args>(args)...);
		}

		template <typename T>
		bool HasComponent() const
		{
			auto signature = m_Scene->m_ECS.GetEntitySignature(m_ID);
			return signature.test(T::GetID());
		}

		void Destroy()
		{
			m_Scene->m_ECS.DestroyEntity(m_ID);
		}

		Entity& operator =(const Entity& other)
		{
			m_ID = other.m_ID;
			m_Scene = other.m_Scene;
			return *this;
		}

		bool operator ==(const Entity& other) const
		{
			return (m_ID == other.m_ID && m_Scene == other.m_Scene);
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