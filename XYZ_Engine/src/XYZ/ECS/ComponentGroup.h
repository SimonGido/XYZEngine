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
			XYZ_ASSERT(it == m_EntityIndexMap.end(), "Entity already belong to the group");
			
			int index = m_IndexGroups.Insert({});
			m_EntityIndexMap.insert({ entity,index });
			
			m_IndexGroups[index].Elements = { m_ECS->GetComponentIndex<T>(entity)... };
		}

		virtual void RemoveEntity(uint32_t entity) override
		{
			auto it = m_EntityIndexMap.find(entity);
			XYZ_ASSERT(it != m_EntityIndexMap.end(), "Entity does not belong to the group");

			m_IndexGroups.Erase(it->second);
			m_EntityIndexMap.erase(it->first);
		}

		auto operator[] (int index)
		{
			return std::tuple<T*...>{ &get<T>(index)... };
		}		
		
		int Size() const { return m_IndexGroups.Range(); }

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

			Element(int index)
				: Index(index)
			{}

			int Index = 0;
		};

		struct IndexGroup
		{
			std::tuple<Element<T>...> Elements;
		};

		ECSManager* m_ECS;	
		FreeList<IndexGroup> m_IndexGroups;
		std::unordered_map<uint32_t, int> m_EntityIndexMap;
		std::tuple <std::shared_ptr<ComponentStorage<T>>...> m_Storages;

		static constexpr size_t sc_ElementsPerGroup = sizeof...(T);
	};
}