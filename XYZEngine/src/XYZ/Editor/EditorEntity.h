#pragma once

#include "EditorScene.h"


namespace XYZ {
	class EditorEntity
	{
	public:
		EditorEntity() 
			:
			m_Scene(nullptr),
			m_ID(NULL_ENTITY)
		{
		}
		EditorEntity(const EditorEntity& other)
			:
			m_Scene(other.m_Scene),
			m_ID(other.m_ID)
		{}

		EditorEntity(uint32_t id, EditorScene* scene)
			:
			m_Scene(scene),
			m_ID(id)
		{}
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

		EditorEntity& operator =(const EditorEntity& other)
		{
			m_Scene = other.m_Scene;
			m_ID = other.m_ID;
			return *this;
		}

		bool operator ==(const EditorEntity& other) const
		{
			return (m_ID == other.m_ID && m_Scene == other.m_Scene);
		}

		operator bool() const
		{
			return m_Scene && m_ID != NULL_ENTITY;
		}

		operator uint32_t () const { return m_ID; }


	private:
		EditorScene* m_Scene;
		uint32_t m_ID;


		friend class EditorScene;
	};
}