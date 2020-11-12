#pragma once
#include "ComponentStorage.h"
#include "ComponentGroup.h"
#include "Component.h"
#include "Types.h"

namespace XYZ {
	namespace ECS {

		class ComponentManager
		{
		public:
			~ComponentManager()
			{
				for (auto storage : m_Storages)
					delete storage;
				
				for (auto group : m_Groups)
					delete group;
			}

			template <typename T>
			T& AddComponent(uint32_t entity, const T& component)
			{
				if (T::GetComponentID() >= m_Storages.size())
				{
					m_Storages.resize(size_t(T::GetComponentID()) + 1);
					m_Storages[T::GetComponentID()] = new ComponentStorage<T>();
				}
				else if (!m_Storages[T::GetComponentID()])
					m_Storages[T::GetComponentID()] = new ComponentStorage<T>();

				ComponentStorage<T>* casted = (ComponentStorage<T>*)m_Storages[T::GetComponentID()];

				return casted->AddComponent(entity, component);
			}

			template <typename T>
			void RemoveComponent(uint32_t entity)
			{
				ComponentStorage<T>* casted = (ComponentStorage<T>*)m_Storages[T::GetComponentID()];
				casted->RemoveComponent(entity);
			}

			template <typename T>
			T& GetComponent(uint32_t entity)
			{
				ComponentStorage<T>* casted = (ComponentStorage<T>*)m_Storages[T::GetComponentID()];
				return casted->GetComponent(entity);
			}

			IComponentStorage* GetStorage(uint8_t index)
			{
				return m_Storages[(size_t)index];
			}

			IComponentGroup* GetGroup(const Signature& signature)
			{
				for (auto group : m_Groups)
				{
					if ((group->GetSignature() & signature) == group->GetSignature())
						return group;
				}
				return nullptr;
			}

			template <typename ...Args>
			ComponentGroup<Args...>* CreateGroup()
			{
				Signature signature;
				std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
				for (auto it : componentTypes)
					signature.set(it);
				
				for (auto group : m_Groups)
				{		
					XYZ_ASSERT(!(group->GetSignature() & signature).any(), "Component is already owned by different group");
				}

				auto group = new ComponentGroup<Args...>();
				m_Groups.push_back(group);
				return group;
			}


			void EntityDestroyed(uint32_t entity, const Signature& signature, ECSManager* ecs)
			{
				for (uint32_t i = 0; i < m_Storages.size(); ++i)
				{
					if (signature.test(i))
					{
						if (m_Storages[i])
							m_Storages[i]->EntityDestroyed(entity);
					}
				}
				for (auto group : m_Groups)
				{
					if (group->GetSignature() == signature)
						group->RemoveEntity(entity, ecs);
				}
			}

		private:
			std::vector<IComponentStorage*> m_Storages;
			std::vector<IComponentGroup*> m_Groups;
		};
	}
}