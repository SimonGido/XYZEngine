#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"

#include <limits>

namespace XYZ {	
	class ECSManager;

	template <typename ...Args>
	class ComponentView
	{
	public:
		std::tuple<Args&...> Get(uint32_t entity)
		{
			return std::tuple<Args&...>{ get<Args>(entity)... };
		}

		template <typename ...Args2>
		std::tuple<Args2&...> Get(uint32_t entity)
		{
			return std::tuple<Args2&...>{ get<Args2>(entity)... };
		}

		std::vector<uint32_t>::const_iterator begin() const { return m_Entities->begin(); }
		std::vector<uint32_t>::const_iterator end()   const { return m_Entities->end(); }

	private:
		ComponentView(ECSManager* ecs)
		{
			(ecs->ForceStorage<Args>(),...);
			m_Storages = { &ecs->GetStorage<Args>()... };

			size_t minimalSize = std::numeric_limits<size_t>::max();
			ForEachInTuple(m_Storages, [&](const auto& storage) {
				if (storage->Size() < minimalSize)
					minimalSize = storage->Size();
					m_Entities = &storage->GetDataEntityMap();
				});
		}

		template <typename Type>
		Type& get(uint32_t entity)
		{
			auto it = std::get<ComponentStorage<Type>*>(m_Storages);
			return it->GetComponent(entity);
		}

	private:
		const std::vector<uint32_t>* m_Entities;
		std::tuple<ComponentStorage<Args>*...> m_Storages;

		friend class ECSManager;
	};
}