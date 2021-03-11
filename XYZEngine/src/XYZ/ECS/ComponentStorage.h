#pragma once
#include "Types.h"

#include "ComponentAllocator.h"

namespace XYZ {

	class ComponentStorage
	{
	public:
		ComponentStorage();
		ComponentStorage(uint8_t id);
		
		uint32_t EntityDestroyed(uint32_t entity)
		{
			uint32_t updatedEntity = NULL_ENTITY;
			if (entity != m_DataEntityMap.back())
			{
				uint32_t lastEntity = m_DataEntityMap.back();
				uint32_t index = m_EntityDataMap[entity];
				m_Data.Copy(index, m_Data.Size() - 1, m_Data.GetElementSize());

				m_DataEntityMap[index] = lastEntity;
				m_EntityDataMap[lastEntity] = index;
				updatedEntity = lastEntity;
			}
			m_Data.Erase(m_Data.Size() - 1, m_Data.GetElementSize());
			return updatedEntity;
		}

		template <typename T, typename ...Args>
		T& EmplaceComponent(uint32_t entity, Args&& ... args)
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);
			
			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.Size();
			return *m_Data.Emplace(std::forward<Args>(args)...);
		}
		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);

			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.Size();
			return *m_Data.Push({ component,entity });
		}
		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			return m_Data.Get(m_EntityDataMap[entity]);
		}

		template <typename T>
		const T& GetComponent(uint32_t entity) const
		{
			return m_Data.Get(m_EntityDataMap[entity]);
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
				m_Data.Move<T>(index, m_Data.Size() - 1);
				// Point data entity map at index to last entity
				m_DataEntityMap[index] = lastEntity;
				// Point last entity to data new index;
				m_EntityDataMap[lastEntity] = index;
				// Pop back last element
				updatedEntity = lastEntity;
			}
			m_Data.Pop();
			return updatedEntity;
		}
		template <typename T>
		ComponentStorage Clone() override
		{
			ComponentStorage storage;
			m_Data.Clone(storage.m_Data);
			storage.m_DataEntityMap = m_DataEntityMap;
			storage.m_EntityDataMap = m_EntityDataMap;
			return storage;
		}

		template <typename T>
		T& GetComponentAtIndex(size_t index)
		{
			return m_Data.Get(index);
		}

		template <typename T>
		const T& GetComponentAtIndex(size_t index) const
		{
			return m_Data.Get(index);
		}

		uint32_t GetComponentIndex(uint32_t entity) const
		{
			return m_EntityDataMap[entity];
		}

		uint32_t GetEntityAtIndex(size_t index) const
		{
			return m_DataEntityMap[index];
		}

		size_t Size() const { return m_Data.Size(); }

		uint8_t GetComponentID() const { return m_Data.GetID(); }
	private:
		ComponentAllocator m_Data;
		std::vector<uint32_t> m_EntityDataMap;
		std::vector<uint32_t> m_DataEntityMap;
	};
}