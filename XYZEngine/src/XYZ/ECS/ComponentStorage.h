#pragma once
#include "Types.h"


namespace XYZ {

	class IComponentStorage
	{
	public:
		virtual uint32_t EntityDestroyed(uint32_t entity) = 0;
	};

	template <typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		virtual uint32_t EntityDestroyed(uint32_t entity) override
		{
			return RemoveComponent<T>(entity);
		}

		template <typename ...Args>
		T& EmplaceComponent(uint32_t entity, Args&& ... args)
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);
			
			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.Size();
			m_Data.emplace_back(std::forward<Args>(args)...);
			return m_Data.back();
		}

		T& AddComponent(uint32_t entity, const T& component)
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);

			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.size();
			m_Data.push_back(component);
			return m_Data.back();
		}

		T& GetComponent(uint32_t entity)
		{
			return m_Data[m_EntityDataMap[entity]];
		}

		const T& GetComponent(uint32_t entity) const
		{
			return m_Data[m_EntityDataMap[entity]];
		}

		template <typename T>
		uint32_t RemoveComponent(uint32_t entity)
		{
			uint32_t updatedEntity = NULL_ENTITY;
			if (entity != m_DataEntityMap.back())
			{
				// Entity of last element in data pack
				uint32_t lastEntity = m_DataEntityMap.back();
				// Index that is entity pointing to
				uint32_t index = m_EntityDataMap[entity];
				// Move last element in data pack at the place of removed component
				m_Data[index] = std::move(m_Data.back());
				// Point data entity map at index to last entity
				m_DataEntityMap[index] = lastEntity;
				// Point last entity to data new index;
				m_EntityDataMap[lastEntity] = index;
				// Pop back last element
				updatedEntity = lastEntity;
			}
			m_Data.pop_back();
			return updatedEntity;
		}

		ComponentStorage<T> Clone() const
		{
			ComponentStorage<T> storage;
			m_Data.Clone<T>(storage.m_Data);
			storage.m_DataEntityMap = m_DataEntityMap;
			storage.m_EntityDataMap = m_EntityDataMap;
			return storage;
		}

		template <typename T>
		T& GetComponentAtIndex(size_t index)
		{
			return m_Data[index];
		}

		template <typename T>
		const T& GetComponentAtIndex(size_t index) const
		{
			return m_Data[index];
		}

		uint32_t GetComponentIndex(uint32_t entity) const
		{
			return m_EntityDataMap[entity];
		}

		uint32_t GetEntityAtIndex(size_t index) const
		{
			return m_DataEntityMap[index];
		}

		size_t Size() const { return m_Data.size();  }

		T& operator[](size_t index)
		{
			return m_Data[index];
		}
		const T& operator[](size_t index) const
		{
			return m_Data[index];
		}

	private:
		std::vector<T> m_Data;
		std::vector<uint32_t> m_EntityDataMap;
		std::vector<uint32_t> m_DataEntityMap;
	};
}