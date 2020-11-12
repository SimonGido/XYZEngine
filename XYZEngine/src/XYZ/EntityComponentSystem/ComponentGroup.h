#pragma once
#include "Component.h"
#include "Types.h"


namespace XYZ {
	namespace ECS {

		template <int I, class... Ts>
		decltype(auto) get(Ts&&... ts)
		{
			return std::get<I>(std::forward_as_tuple(ts...));
		}
		template<typename... Types>
		constexpr auto GetTypesSize()
		{
			return std::array<std::size_t, sizeof...(Types)>{sizeof(Types)...};
		}

		template<class F, class...Ts, std::size_t...Is>
		void for_each_in_tuple(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>) 
		{
			using expander = int[];
			(void)expander {
				0, ((void)func(std::get<Is>(tuple)), 0)...
			};
		}

		template<class F, class...Ts>
		void for_each_in_tuple(const std::tuple<Ts...>& tuple, F func) 
		{
			for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
		}

		class ECSManager;

		class IComponentGroup
		{		
		public:
			virtual void AddEntity(uint32_t entity, Signature& signature, ECSManager* ecs) = 0;
			virtual void RemoveEntity(uint32_t entity, ECSManager* ecs) = 0;
			virtual bool HasEntity(uint32_t entity) = 0;

			virtual void FindComponent(uint8_t** object, uint32_t entity, uint8_t id) = 0;

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

			virtual void RemoveEntity(uint32_t entity, ECSManager* ecs) override
			{
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
				}
				m_Data.pop_back();
			}

			virtual void FindComponent(uint8_t** object, uint32_t entity, uint8_t id)
			{
				uint32_t counter = 0;
				for_each_in_tuple(m_Data[m_EntityDataMap[entity]].Data, [&](const auto& x) { 
					if (m_MemoryLayout[counter++].ID == id)
					{
						*object = (uint8_t*)&x;
						return;
					}
				});
			}

			virtual bool HasEntity(uint32_t entity) override
			{
				return m_EntityDataMap.size() > entity && m_Data.size() > m_EntityDataMap[entity];
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
				uint32_t Entity;
			};

			std::vector<Pack> m_Data;
			std::vector<uint32_t> m_EntityDataMap;

			static constexpr size_t sc_NumElements = sizeof...(Args);

			struct MemoryLayout
			{
				size_t Size;
				uint8_t ID;
			};
			
			MemoryLayout m_MemoryLayout[sc_NumElements];
		};
	}
}