#pragma once



#include "ECSManager.h"

namespace XYZ {

	class Entity
	{
	public:
		Entity()
		{
			m_ID = ECSManager::CreateEntity();
		}

		template<typename T>
		T* GetComponent()
		{
			return ECSManager::GetComponent<T>(m_ID);
		}

		template <typename T>
		T* AddComponent(const T& component)
		{
			return ECSManager::AddComponent<T>(m_ID, component);
		}

		template <typename T>
		bool HasComponent()
		{
			return ECSManager::Contains<T>(m_ID);
		}

		uint32_t GetID()
		{
			return m_ID;
		}

		void Destroy()
		{
			ECSManager::DestroyEntity(m_ID);
		}

	private:
		uint32_t m_ID;
	};
}