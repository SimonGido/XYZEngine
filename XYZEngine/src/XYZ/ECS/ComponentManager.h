#pragma once
#include "ComponentStorage.h"
#include "ComponentGroup.h"
#include "ComponentView.h"
#include "Component.h"
#include "Types.h"

namespace XYZ {

	class ComponentManager
	{
	public:
		~ComponentManager()
		{
			for (auto storage : m_Storages)
			{
				if (storage)
					delete storage;
			}
			for (auto group : m_Groups)
			{
				if (group)
					delete group;
			}
			for (auto view : m_Views)
			{
				if (view)
					delete view;
			}
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
		void ForceStorage()
		{
			if (T::GetComponentID() >= m_Storages.size())
			{
				m_Storages.resize(size_t(T::GetComponentID()) + 1);
				m_Storages[T::GetComponentID()] = new ComponentStorage<T>();
			}
			else if (!m_Storages[T::GetComponentID()])
				m_Storages[T::GetComponentID()] = new ComponentStorage<T>();
		}

		void AddRawComponent(uint32_t entity, uint8_t* component, uint8_t componentID)
		{
			m_Storages[componentID]->AddRawComponent(entity, component);
		}

		void AddToGroup(uint32_t entity, Signature& signature, ECSManager* ecs)
		{
			for (auto group : m_Groups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature()
				 && !group->HasEntity(entity))
				{
					group->AddEntity(entity, signature, ecs);
				}
			}
		}

		void AddToView(uint32_t entity,const Signature& signature)
		{
			for (auto view : m_Views)
			{
				if ((view->GetSignature() & signature) == view->GetSignature() && !view->HasEntity(entity))
				{
					view->AddEntity(entity);
				}
			}
		}

		void RemoveFromGroup(uint32_t entity, uint8_t id, Signature& signature, ECSManager* ecs)
		{
			for (auto group : m_Groups)
			{
				auto& groupSignature = group->GetSignature();
				if ((groupSignature & signature) == groupSignature && group->HasEntity(entity))
				{
					signature.set(id, false);
					
					auto memoryLayout = group->GetMemoryLayout();
					auto numElements = group->GetNumberOfElements();
					// Rest of the elements that no longer belong to group are removed and copied to the separate storages
					for (size_t i = 0; i < numElements; ++i)
					{
						if (signature.test(memoryLayout[i].ID))
						{
							uint8_t* component = nullptr;
							group->FindComponent(&component, entity, memoryLayout[i].ID);
							AddRawComponent(entity, component, memoryLayout[i].ID);
						}
					}
					group->RemoveEntity(entity);
					return;
				}
			}
		}

		void RemoveFromView(uint32_t entity)
		{
			for (auto view : m_Views)
			{
				if (view->HasEntity(entity))
					view->RemoveEntity(entity);
			}
		}

		void GetFromGroup(uint32_t entity,const Signature& signature, uint8_t componentID, uint8_t** component)
		{
			for (auto group : m_Groups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature()
					&& group->HasEntity(entity))
				{
					group->FindComponent(component, entity, componentID);
					return;
				}
			}
		}
		void GetFromGroup(uint32_t entity, const Signature& signature, uint8_t componentID, uint8_t** component) const
		{
			for (auto group : m_Groups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature()
					&& group->HasEntity(entity))
				{
					group->FindComponent(component, entity, componentID);
					return;
				}
			}
		}
		template <typename T>
		void RemoveComponent(uint32_t entity, const Signature& signature)
		{
			ComponentStorage<T>* casted = (ComponentStorage<T>*)m_Storages[T::GetComponentID()];
			uint32_t updatedEntity = casted->RemoveComponent(entity);
			if (updatedEntity != NULL_ENTITY)
				updateViews(entity,signature, updatedEntity);
		}

		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			ComponentStorage<T>* casted = (ComponentStorage<T>*)m_Storages[T::GetComponentID()];
			return casted->GetComponent(entity);
		}
		template <typename T>
		T& GetComponent(uint32_t entity) const
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
		IComponentView* GetView(const Signature& signature)
		{
			for (auto view : m_Views)
			{
				if ((view->GetSignature() & signature) == view->GetSignature())
					return view;
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

		template <typename ...Args>
		ComponentView<Args...>* CreateView(ECSManager * ecs)
		{
			Signature signature;
			std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
			for (auto it : componentTypes)
				signature.set(it);
			for (auto view : m_Views)
			{
				XYZ_ASSERT(view->GetSignature() != signature, "View already exists");
			}
			auto view = new ComponentView<Args...>(ecs);
			m_Views.push_back(view);
			return view;
		}

		uint32_t GetComponentIndex(uint32_t entity, uint32_t index) const
		{
			return m_Storages[index]->GetComponentIndex(entity);
		}

		void EntityDestroyed(uint32_t entity, const Signature& signature, ECSManager* ecs)
		{
			std::vector<uint32_t> updated;
			for (uint32_t i = 0; i < m_Storages.size(); ++i)
			{
				if (signature.test(i) && m_Storages[i])
					updated.push_back(m_Storages[i]->EntityDestroyed(entity));	
			}
			for (auto group : m_Groups)
			{
				if (group->HasEntity(entity))
					group->RemoveEntity(entity);
			}
			updateViews(entity, signature, updated);
		}

	private:
		// This is called when entity index in storage has changed
		void updateViews(uint32_t removedEntity, const Signature& signature, uint32_t updatedEntity)
		{
			for (auto view : m_Views)
			{
				if (view->HasEntity(removedEntity) && (view->GetSignature() & signature) == view->GetSignature())
					view->RemoveEntity(removedEntity);

				if (view->HasEntity(updatedEntity))
					view->EntityComponentUpdated(updatedEntity);
			}
		}
		void updateViews(uint32_t removedEntity, const Signature& signature, const std::vector<uint32_t>& updatedEntities)
		{
			for (auto view : m_Views)
			{
				if (view->HasEntity(removedEntity) && (view->GetSignature() & signature) == view->GetSignature())
					view->RemoveEntity(removedEntity);

				for (auto updatedEntity : updatedEntities)
				{
					if (view->HasEntity(updatedEntity))
						view->EntityComponentUpdated(updatedEntity);
				}
			}
		}

	private:
		std::vector<IComponentStorage*> m_Storages;
		std::vector<IComponentGroup*> m_Groups;
		std::vector<IComponentView*> m_Views;
	};
	
}