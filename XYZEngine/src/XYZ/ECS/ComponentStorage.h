#pragma once
#include "Types.h"
#include "Entity.h"
#include "Serialization/ByteStream.h"

namespace XYZ {

	class IComponentStorage
	{
	public:
		virtual					  ~IComponentStorage() = default;
		virtual void			   Clear() = 0;
		virtual void			   Move(uint8_t* buffer) = 0;
		virtual void			   CopyComponentData(Entity entity, ByteStream& out) const = 0;
		virtual void			   UpdateComponentData(Entity entity, const ByteStream& in) = 0;
		virtual Entity			   EntityDestroyed(Entity entity) = 0;
		virtual uint32_t		   GetComponentIndex(Entity entity) const = 0;
		virtual Entity			   GetEntityAtIndex(size_t index) const = 0;
	
		virtual size_t			   Size() const = 0;
		virtual uint8_t			   ID() const = 0;
		virtual IComponentStorage* Copy(uint8_t* buffer) const = 0;

		virtual const std::vector<Entity>& GetDataEntityMap() const = 0;
	};

	template <typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		ComponentStorage() = default;
		ComponentStorage(const ComponentStorage<T>& other)
			: 
			m_Data(other.m_Data),
			m_DataEntityMap(other.m_DataEntityMap),
			m_EntityDataMap(other.m_EntityDataMap)
		{}
		ComponentStorage(ComponentStorage<T>&& other) noexcept
			:
			m_Data(std::move(other.m_Data)),
			m_DataEntityMap(std::move(other.m_DataEntityMap)),
			m_EntityDataMap(std::move(other.m_EntityDataMap))
		{}

		virtual void Clear() override
		{
			m_DataEntityMap.clear();
			m_EntityDataMap.clear();
			m_Data.clear();
		}
		virtual void Move(uint8_t* buffer) override
		{
			new (buffer)ComponentStorage<T>(std::move(*this));
		}
		virtual void CopyComponentData(Entity entity, ByteStream& out) const override
		{
			out << m_Data[m_EntityDataMap[(uint32_t)entity]];
		}
		virtual void UpdateComponentData(Entity entity, const ByteStream& in) override
		{
			in >> m_Data[m_EntityDataMap[(uint32_t)entity]];
		}
		virtual Entity EntityDestroyed(Entity entity) override
		{
			return RemoveComponent(entity);
		}
		virtual uint32_t GetComponentIndex(Entity entity) const override
		{
			return m_EntityDataMap[(uint32_t)entity];
		}
		virtual Entity GetEntityAtIndex(size_t index) const override
		{
			return m_DataEntityMap[index];
		}
		virtual size_t Size() const override 
		{ 
			return m_Data.size();  
		}
		virtual uint8_t ID() const override
		{
			return IComponent::GetComponentID<T>();
		}
		virtual IComponentStorage* Copy(uint8_t* buffer) const override
		{
			return new ((void*)buffer)ComponentStorage<T>(*this);
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

		friend class ECSSerializer;
	};
}