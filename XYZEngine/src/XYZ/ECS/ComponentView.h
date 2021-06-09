#pragma once
#include "ComponentStorage.h"
#include "ComponentManager.h"
#include "Component.h"
#include "Types.h"
#include "Entity.h"

#include <limits>

namespace XYZ {	
	template <typename ...Args>
	class ComponentView
	{
	public:
		std::tuple<Args&...> Get(Entity entity)
		{
			return std::tuple<Args&...>{ get<Args>(entity)... };
		}

		template <typename ...Args2>
		std::tuple<Args2&...> Get(Entity entity)
		{
			return std::tuple<Args2&...>{ get<Args2>(entity)... };
		}

		std::vector<Entity>::const_iterator begin() const { return m_Entities->begin(); }
		std::vector<Entity>::const_iterator end()   const { return m_Entities->end(); }

	private:
		ComponentView(ComponentManager& componentManager)
		{
			m_Storages = { &componentManager.GetStorage<Args>()... };

			size_t minimalSize = std::numeric_limits<size_t>::max();
			ForEachInTuple(m_Storages, [&](const auto& storage) {
				if (storage->Size() < minimalSize)
					minimalSize = storage->Size();
					m_Entities = &storage->GetDataEntityMap();
				});
		}

		template <typename Type>
		Type& get(Entity entity)
		{
			auto it = std::get<ComponentStorage<Type>*>(m_Storages);
			return it->GetComponent(entity);
		}

	private:
		const std::vector<Entity>* m_Entities;
		std::tuple<ComponentStorage<Args>*...> m_Storages;

		friend class ECSManager;
	};
}