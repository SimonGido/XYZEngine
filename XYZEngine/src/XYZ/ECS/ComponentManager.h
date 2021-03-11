#pragma once
#include "ComponentStorage.h"
#include "ComponentView.h"
#include "Component.h"
#include "Types.h"

namespace XYZ {

	class ComponentManager
	{
	public:
		~ComponentManager()
		{
			for (auto view : m_Views)
			{
				if (view)
					delete view;
			}
		}

		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			uint8_t id = T::GetComponentID();
			if (id >= m_Storages.size())
			{
				m_Storages.resize(size_t(id) + 1);
				m_Storages[id] = ComponentStorage(id);
			}
			else if (!m_Storages[id].GetComponentID() )
				m_Storages[id] = ComponentStorage(id);

			return m_Storages[id].AddComponent<T>(entity, component);
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

	
		void RemoveFromView(uint32_t entity)
		{
			for (auto view : m_Views)
			{
				if (view->HasEntity(entity))
					view->RemoveEntity(entity);
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

		template <typename T>
		ComponentStorage& GetStorage()
		{
			return m_Storages[(size_t)T::GetComponentID()];
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
			return m_Storages[index].GetComponentIndex(entity);
		}

		size_t GetNumberOfStorages() const { return m_Storages.size(); }

		void EntityDestroyed(uint32_t entity, const Signature& signature)
		{
			std::vector<uint32_t> updated;
			for (uint32_t i = 0; i < m_Storages.size(); ++i)
			{
				if (signature.test(i) && m_Storages[i].GetComponentID() != INVALID_COMPONENT)
					updated.push_back(m_Storages[i].EntityDestroyed(entity));	
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
		std::vector<ComponentStorage> m_Storages;
		std::vector<IComponentView*> m_Views;
	};
	
}