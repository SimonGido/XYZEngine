#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"
#include "Pool.h"

namespace XYZ {

	class ComponentManager
	{
	public:
		ComponentManager();	
		ComponentManager(const ComponentManager& other);

		~ComponentManager();

		void EntityDestroyed(uint32_t entity, const Signature& signature);	


		template <typename T, typename ...Args>
		T& EmplaceComponent(uint32_t entity, Args&& ... args)
		{
			ComponentStorage<T>* storage = getOrCreateStorage<T>();
			return storage->EmplaceComponent(entity, std::forward<Args>(args)...);
		}

		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			ComponentStorage<T>* storage = getOrCreateStorage<T>();
			return storage->AddComponent(entity, component);
		}


		template <typename T>
		void ForceStorage()
		{
			getOrCreateStorage<T>();
		}	

		template <typename T>
		void RemoveComponent(uint32_t entity, const Signature& signature)
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			uint32_t updatedEntity = m_StoragePool.Get<ComponentStorage<T>>(offset)->RemoveComponent(entity);
		}

		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			ComponentStorage<T>* storage = static_cast<ComponentStorage<T>*>(m_StoragePool.Get<IComponentStorage>(offset));
			return storage->GetComponent(entity);
		}

		template <typename T>
		const T& GetComponent(uint32_t entity) const
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			const ComponentStorage<T>* storage = static_cast<const ComponentStorage<T>*>(m_StoragePool.Get<IComponentStorage>(offset));
			return storage->GetComponent(entity);
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

		const IComponentStorage* GetIStorage(size_t offset) const
		{
			return m_StoragePool.Get<IComponentStorage>(offset);
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
				m_StoragePool.Allocate<ComponentStorage<T>>((size_t)id * sizeof(ComponentStorage<T>));
				m_StorageCreated[id] = true;
				m_NumberOfStorages++;
			}
			return m_StoragePool.Get<ComponentStorage<T>>((size_t)id * sizeof(ComponentStorage<T>));
		}
	
	private:
		Pool m_StoragePool;
		std::vector<bool> m_StorageCreated;
		size_t m_NumberOfStorages = 0;
	};
	
}