#pragma once
#include "Types.h"

namespace XYZ {

	namespace ECS {
		class IComponentStorage
		{
			friend class ComponentManager;
		public:
			/**
			* virtual destructor
			*/
			virtual ~IComponentStorage() = default;
			virtual void AddRawComponent(uint32_t entity, uint8_t* component) = 0;
			virtual void EntityDestroyed(uint32_t entity) = 0;
			virtual uint32_t GetComponentIndex(uint32_t entity) const = 0;
		};


		template <typename T>
		class ComponentStorage : public IComponentStorage
		{
		public:
			T& AddComponent(uint32_t entity, const T& component)
			{
				if (m_EntityDataMap.size() <= entity)
					m_EntityDataMap.resize(size_t(entity) + 1);

				m_EntityDataMap[entity] = m_Data.size();

				m_Data.push_back({ component,entity });

				return m_Data[m_EntityDataMap[entity]].Data;
			}

			T& GetComponent(uint32_t entity)
			{
				return m_Data[m_EntityDataMap[entity]].Data;
			}

			
			uint32_t RemoveComponent(uint32_t entity)
			{
				uint32_t updatedEntity = NULL_ENTITY;
				if (m_Data.size() > 1)
				{
					// Entity of last element in data pack
					uint32_t lastEntity = m_Data.back().Entity;
					// Index that is entity pointing to
					uint32_t index = m_EntityDataMap[entity];
					// Move last element in data pack at the place of removed component
					m_Data[index] = std::move(m_Data.back());
					// Point last entity to data new index;
					m_EntityDataMap[lastEntity] = index;
					// Pop back last element
					updatedEntity = lastEntity;
				}
				m_Data.pop_back();
				return updatedEntity;
			}

			virtual uint32_t GetComponentIndex(uint32_t entity) const override
			{
				return m_EntityDataMap[entity];
			}

			virtual void AddRawComponent(uint32_t entity, uint8_t* component) override
			{
				AddComponent(entity, *(T*)component);
			}
			virtual void EntityDestroyed(uint32_t entity) override
			{
				RemoveComponent(entity);
			}

			T& operator[](size_t index)
			{
				return m_Data[index].Data;
			}

			const T& operator[](size_t index) const
			{
				return m_Data[index].Data;
			}

		private:
			struct Pack
			{
				T Data;
				uint32_t Entity;
			};

			std::vector<Pack> m_Data;
			std::vector<uint32_t> m_EntityDataMap;
		};
	}
}