#pragma once
#include "Types.h"
#include "Entity.h"

namespace XYZ {

	class IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;
		virtual Entity EntityDestroyed(Entity entity) = 0;
		virtual size_t Size() const = 0;
		virtual IComponentStorage* Copy(uint8_t* buffer) const = 0;
		

		virtual const std::vector<Entity>& GetDataEntityMap() const = 0;
	};

	template <typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		virtual Entity EntityDestroyed(Entity entity) override
		{
			return RemoveComponent(entity);
		}
		virtual size_t Size() const override 
		{ 
			return m_Data.size();  
		}
		virtual IComponentStorage* Copy(uint8_t* buffer) const override
		{
			ComponentStorage<T>* clone = new ((void*)buffer)ComponentStorage<T>();
			clone->m_Data = m_Data;
			clone->m_DataEntityMap = m_DataEntityMap;
			clone->m_EntityDataMap = m_EntityDataMap;
			return clone;
		}
		virtual const std::vector<Entity>& GetDataEntityMap() const override
		{ 
			return m_DataEntityMap; 
		}

		template <typename ...Args>
		T& EmplaceComponent(Entity entity, Args&& ... args)
		{
			if (m_EntityDataMap.size() <= (size_t)entity)
				m_EntityDataMap.resize((size_t)entity + 1);
			
			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[(size_t)entity] = m_Data.size();
			m_Data.emplace_back(std::forward<Args>(args)...);
			return m_Data.back();
		}

		T& AddComponent(Entity entity, const T& component)
		{
			if (m_EntityDataMap.size() <= (size_t)entity)
				m_EntityDataMap.resize((size_t)entity + 1);

			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[(size_t)entity] = m_Data.size();
			m_Data.push_back(component);
			return m_Data.back();
		}

		T& GetComponent(Entity entity)
		{
			return m_Data[m_EntityDataMap[(uint32_t)entity]];
		}

		const T& GetComponent(Entity entity) const
		{
			return m_Data[m_EntityDataMap[(uint32_t)entity]];
		}


		uint32_t RemoveComponent(Entity entity)
		{
			Entity updatedEntity;
			if (entity != m_DataEntityMap.back())
			{
				// Entity of last element in data pack
				Entity lastEntity = m_DataEntityMap.back();
				// Index that is entity pointing to
				uint32_t index = m_EntityDataMap[(uint32_t)entity];
				// Move last element in data pack at the place of removed component
				m_Data[index] = std::move(m_Data.back());
				// Point data entity map at index to last entity
				m_DataEntityMap[index] = lastEntity;
				// Point last entity to data new index;
				m_EntityDataMap[(uint32_t)lastEntity] = index;
				// Pop back last element
				updatedEntity = lastEntity;
			}
			m_Data.pop_back();
			m_DataEntityMap.pop_back();
			return updatedEntity;
		}

		T& GetComponentAtIndex(size_t index)
		{
			return m_Data[index];
		}

		const T& GetComponentAtIndex(size_t index) const
		{
			return m_Data[index];
		}

		uint32_t GetComponentIndex(Entity entity) const
		{
			return m_EntityDataMap[(uint32_t)entity];
		}

		Entity GetEntityAtIndex(size_t index) const
		{
			return m_DataEntityMap[index];
		}

		T& operator[](size_t index)
		{
			return m_Data[index];
		}
		const T& operator[](size_t index) const
		{
			return m_Data[index];
		}

		
		
		typename std::vector<T>::iterator begin() { return m_Data.begin(); }
		typename std::vector<T>::iterator end() { return m_Data.end(); }
		typename std::vector<T>::const_iterator begin() const { return m_Data.begin(); }
		typename std::vector<T>::const_iterator end()   const { return m_Data.end(); }

	private:
		std::vector<T> m_Data;
		std::vector<Entity> m_DataEntityMap;
		std::vector<uint32_t> m_EntityDataMap;
	};
}