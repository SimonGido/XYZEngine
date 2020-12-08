#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"



namespace XYZ {	

	class IComponentView
	{
	public:
		virtual void AddEntity(uint32_t entity) = 0;
		virtual void RemoveEntity(uint32_t entity) = 0;
		virtual void EntityComponentUpdated(uint32_t entity) = 0;
		virtual bool HasEntity(uint32_t entity) const = 0;

		const Signature& GetSignature() const { return m_Signature; }

	protected:
		Signature m_Signature;
	};


	class ECSManager;

	template <typename ...Args>
	class ComponentView : public IComponentView
	{
	public:
		ComponentView(ECSManager* ecs)
			:
			m_ECS(ecs)
		{
			std::initializer_list<uint8_t> componentTypes{ Args::GetComponentID()... };
			for (auto it : componentTypes)
				m_Signature.set(it);

			(m_ECS->ForceStorage<Args>(),...);
			m_Storages = { m_ECS->GetStorage<Args>()... };
		}

		virtual void AddEntity(uint32_t entity) override
		{
			if (m_EntityDataMap.size() <= entity)
				m_EntityDataMap.resize(size_t(entity) + 1);

			m_IndexGroups.push_back({});
			uint32_t index = m_IndexGroups.size() - 1;
			m_EntityDataMap[entity] = index;
			m_IndexGroups[index].Entity = entity;
			m_IndexGroups[index].Elements = { m_ECS->GetComponentIndex<Args>(entity)... };
		}


		virtual void RemoveEntity(uint32_t entity) override
		{
			if (m_IndexGroups.back().Entity != entity)
			{
				// Entity of last element in data pack
				uint32_t lastEntity = m_IndexGroups.back().Entity;
				// Index that is entity pointing to
				uint32_t index = m_EntityDataMap[entity];
				// Move last element in data pack at the place of removed component
				m_IndexGroups[index] = std::move(m_IndexGroups.back());
				// Point last entity to data new index;
				m_EntityDataMap[lastEntity] = index;
				// Pop back last element
				
				m_IndexGroups[index].Elements = { m_ECS->GetComponentIndex<Args>(m_IndexGroups[index].Entity)... };
			}
			m_IndexGroups.pop_back();
		}


		virtual void EntityComponentUpdated(uint32_t entity) override
		{
			m_IndexGroups[m_EntityDataMap[entity]].Elements = { m_ECS->GetComponentIndex<Args>(entity)... };
		}

		virtual bool HasEntity(uint32_t entity) const override
		{
			return m_EntityDataMap.size() > entity && m_IndexGroups.size() > m_EntityDataMap[entity]
				&& m_IndexGroups[m_EntityDataMap[entity]].Entity == entity;
		}

		uint32_t GetEntity(size_t index) const
		{
			return m_IndexGroups[index].Entity;
		}

		std::tuple<Args&...> operator[] (size_t index)
		{
			return std::tuple<Args&...>{ get<Args>(index)... };
		}

		size_t Size() const { return m_IndexGroups.size(); }

	private:

		template <typename Type>
		Type& get(uint32_t index)
		{
			auto it = std::get<Element<Type>>(m_IndexGroups[index].Elements);
			return (*std::get<ComponentStorage<Type>*>(m_Storages))[it.Index];
		}

	private:
		template <typename T>
		struct Element
		{
			Element() = default;
			Element(uint32_t index)
				: Index(index)
			{}
			uint32_t Index = 0;
		};
		struct IndexGroup
		{
			std::tuple<Element<Args>...> Elements;
			uint32_t Entity;
		};

		ECSManager* m_ECS;
		std::vector<IndexGroup> m_IndexGroups;
		std::vector<uint32_t> m_EntityDataMap;
		std::tuple<ComponentStorage<Args>*...> m_Storages;

		static constexpr size_t sc_ElementsPerGroup = sizeof...(Args);
	};
}