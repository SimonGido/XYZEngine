#pragma once
#include "Types.h"
#include "ComponentStorage.h"

#include "XYZ/Utils/DataStructures/FreeList.h"

namespace XYZ {

	class ECSManager;

	template <int I, class... Ts>
	decltype(auto) get(Ts&&... ts) 
	{
		return std::get<I>(std::forward_as_tuple(ts...));
	}

	class IComponentGroup
	{
	public:
		virtual ~IComponentGroup() {};

		virtual void AddEntity(uint32_t entity) = 0;
		virtual void RemoveEntity(uint32_t entity) = 0;

		Signature GetSignature() const { return m_Signature; }
	protected:
		Signature m_Signature;
	};

	template <typename ...T>
	class ComponentGroup : public IComponentGroup
	{
	public:
		ComponentGroup(ECSManager* ecs)
			:
			m_ECS(ecs)
		{
			std::initializer_list<uint16_t> componentTypes{ T::GetID()... };
			for (auto it : componentTypes)
				m_Signature.set(it);
			
			
			m_Storages = { m_ECS->GetComponentStorage<T>()... };
		}

		virtual void AddEntity(uint32_t entity) override
		{
			auto it = m_EntityIndexMap.find(entity);
			if (it == m_EntityIndexMap.end())
			{
				m_IndexGroups.push_back({});
				uint32_t index = m_IndexGroups.size() - 1;
				m_EntityIndexMap.insert({ entity,index });

				m_IndexGroups[index].Entity = entity;
				m_IndexGroups[index].Elements = { m_ECS->GetComponentIndex<T>(entity)... };
			}
		}

		virtual void RemoveEntity(uint32_t entity) override
		{
			auto it = m_EntityIndexMap.find(entity);
			if (it != m_EntityIndexMap.end())
			{
				if (it->second != m_IndexGroups.size() - 1)
				{
					m_IndexGroups[it->second] = std::move(m_IndexGroups.back());
					m_IndexGroups.pop_back();
					uint32_t movedEntity = m_IndexGroups[it->second].Entity;
					m_EntityIndexMap[movedEntity] = it->second;
					m_EntityIndexMap.erase(it);
				}
				else
				{
					m_IndexGroups.pop_back();
					m_EntityIndexMap.erase(it);
				}
			}
		}

		auto operator[] (size_t index)
		{
			return std::tuple<T*...>{ &get<T>(index)... };
		}		
		
		size_t Size() const { return m_IndexGroups.size(); }

	private:

		template <typename Type>
		Type& get(int index)
		{
			auto it = std::get<Element<Type>>(m_IndexGroups[index].Elements);
			return (*std::get<std::shared_ptr<ComponentStorage<Type>>>(m_Storages))[it.Index];
		}

	private:
		template <typename T>
		struct Element
		{
			Element() = default;

			Element(int32_t index)
				: Index(index)
			{}

			int32_t Index = 0;
		};

		struct IndexGroup
		{
			std::tuple<Element<T>...> Elements;
			uint32_t Entity;
		};

		ECSManager* m_ECS;	
		std::vector<IndexGroup> m_IndexGroups;
		std::unordered_map<uint32_t, uint32_t> m_EntityIndexMap;
		std::tuple <std::shared_ptr<ComponentStorage<T>>...> m_Storages;

		static constexpr size_t sc_ElementsPerGroup = sizeof...(T);
	};
}