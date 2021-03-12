#pragma once
#include "ComponentStorage.h"
#include "ComponentView.h"
#include "Component.h"
#include "Types.h"
#include "Pool.h"

namespace XYZ {

	class ComponentManager
	{
	public:
		ComponentManager();	
		~ComponentManager();

		void EntityDestroyed(uint32_t entity, const Signature& signature);	


		template <typename T, typename ...Args>
		T& EmplaceComponent(uint32_t entity, Args&& ... args)
		{
			ComponentStorage<T>* storage = getOrCreateStorage<T>();
			return storage->EmplaceComponent<T>(entity, std::forward<Args>(args)...);
		}

		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			ComponentStorage<T>* storage = getOrCreateStorage<T>();
			return storage->AddComponent<T>(entity, component);
		}


		template <typename T>
		void ForceStorage()
		{
			getOrCreateStorage<T>();
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
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			uint32_t updatedEntity = m_StoragePool.Get<ComponentStorage<T>>(offset)->RemoveComponent(entity);
			if (updatedEntity != NULL_ENTITY)
				updateViews(entity,signature, updatedEntity);
		}

		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			ComponentStorage<T>& storage = *m_StoragePool.Get<ComponentStorage<T>>(offset);
			return storage.GetComponent<T>(entity);
		}

		template <typename T>
		const T& GetComponent(uint32_t entity) const
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			return m_StoragePool.Get<ComponentStorage<T>>(offset)->GetComponent<T>(entity);
		}

		template <typename T>
		ComponentStorage<T>& GetStorage()
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			return *m_StoragePool.Get<ComponentStorage<T>>(offset);
		}

		template <typename T>
		const ComponentStorage<T>& GetStorage() const
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			return *m_StoragePool.Get<ComponentStorage<T>>(offset);
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
			std::initializer_list<uint16_t> componentTypes{ IComponent::GetComponentID<Args>()... };
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

		template <typename T>
		uint32_t GetComponentIndex(uint32_t entity) const
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			const ComponentStorage<T>& storage = *m_StoragePool.Get<ComponentStorage<T>>(offset);
			return storage.GetComponentIndex(entity);
		}

		size_t GetNumberOfRegisteredStorages() const { return m_NumberOfStorages; }

	private:
		template <typename T>
		ComponentStorage<T>* getOrCreateStorage()
		{
			uint8_t id = IComponent::GetComponentID<T>();
			if (!m_StorageCreated[id])
			{
				storage = m_StoragePool.Allocate<ComponentStorage<T>>(id * sizeof(ComponentStorage<T>));
				m_StorageCreated[id] = true;
				m_NumberOfStorages++;
			}
			else
			{
				storage = m_StoragePool.Get<ComponentStorage<T>>(id * sizeof(ComponentStorage<T>));
			}
			return storage;
		}
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
		Pool m_StoragePool;
		std::vector<bool> m_StorageCreated;
		size_t m_NumberOfStorages = 0;

		std::vector<IComponentView*> m_Views;
	};
	
}