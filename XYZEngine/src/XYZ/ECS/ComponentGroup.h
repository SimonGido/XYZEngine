#pragma once
#include "Component.h"
#include "Types.h"


namespace XYZ {

	class ECSManager;

	class IComponentGroup
	{		
	public:
		struct MemoryLayout
		{
			size_t Size;
			uint8_t ID;
		};
	public:
		virtual void AddEntity(uint32_t entity, Signature& signature, ECSManager* ecs) = 0;
		virtual void RemoveEntity(uint32_t entity) = 0;
		virtual bool HasEntity(uint32_t entity) const = 0;

		virtual void FindComponent(uint8_t** object, uint32_t entity, uint8_t id) = 0;

		virtual const size_t GetNumberOfElements() const = 0;
		virtual const MemoryLayout* GetMemoryLayout() const = 0;
		const Signature& GetSignature() const { return m_Signature; }
	protected:
		Signature m_Signature;
	};

	template <typename ...Args>
	class ComponentGroup : public IComponentGroup
	{
	public:
		ComponentGroup()
		{
			std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
			uint32_t counter = 0;
			for (auto it : componentTypes)
			{
				m_Signature.set(it);
				m_MemoryLayout[counter++].ID = it;
			}
			counter = 0;
			for (auto it : GetTypesSize<Args...>())
				m_MemoryLayout[counter++].Size = it;
		}

		virtual void AddEntity(uint32_t entity, Signature& signature, ECSManager* ecs) override
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);
			
			m_EntityDataMap[entity] = m_Data.size();

			std::tuple<Args...> tmp = { ecs->GetComponent<Args>(entity)... };
			m_Data.push_back({ tmp, entity });
			(void)std::initializer_list<int>{ ecs->RemoveComponent<Args>(entity)... };
			signature = m_Signature;
			signature.set(HAS_GROUP_BIT);
		}

		virtual void RemoveEntity(uint32_t entity) override
		{
			if (entity != m_Data.back().Entity)
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
			}
			m_Data.pop_back();
		}

		virtual void FindComponent(uint8_t** object, uint32_t entity, uint8_t id)
		{
			uint32_t counter = 0;
			ForEachInTuple(m_Data[m_EntityDataMap[entity]].Data, [&](const auto& x) {
				if (m_MemoryLayout[counter++].ID == id)
				{
					*object = (uint8_t*)&x;
					return;
				}
			});
		}
		virtual bool HasEntity(uint32_t entity) const override
		{
			return m_EntityDataMap.size() > entity && m_Data.size() > m_EntityDataMap[entity] 
			    && m_Data[m_EntityDataMap[entity]].Entity == entity;
		}
		virtual const size_t GetNumberOfElements() const override
		{
			return sc_NumElements;
		}

		virtual const MemoryLayout* GetMemoryLayout() const override
		{
			return m_MemoryLayout;
		}

		std::tuple<Args...>& GetComponents(uint32_t entity)
		{
			return m_Data[m_EntityDataMap[entity]].Data;
		}

		template <typename T>
		T& Get(uint32_t entity)
		{
			return std::get<T>( m_Data[m_EntityDataMap[entity]].Data);
		}

		auto operator[] (size_t index)
		{
			return m_Data[index].Data;
		}
		const auto operator[] (size_t index) const
		{
			return m_Data[index].Data;
		}

		

	private:
		struct Pack
		{
			std::tuple<Args...> Data;
			uint32_t Entity = NULL_ENTITY;
		};

		std::vector<Pack> m_Data;
		std::vector<uint32_t> m_EntityDataMap;

		static constexpr size_t sc_NumElements = sizeof...(Args);

		
		MemoryLayout m_MemoryLayout[sc_NumElements];
	};
}