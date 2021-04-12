#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"
#include "Entity.h"
#include "Pool.h"
#include "Signature.h"

namespace XYZ {

	class ComponentManager
	{
	public:
		ComponentManager();	
		ComponentManager(const ComponentManager& other);
		ComponentManager(ComponentManager&& other) noexcept;
		~ComponentManager();

		ComponentManager& operator=(ComponentManager&& other) noexcept;

		void EntityDestroyed(Entity entity, const Signature& signature);	
		void Clear();

		template <typename T, typename ...Args>
		T& EmplaceComponent(Entity entity, Args&& ... args)
		{
			ComponentStorage<T>& storage = GetStorage<T>();
			return storage.EmplaceComponent(entity, std::forward<Args>(args)...);
		}

		template <typename T>
		T& AddComponent(Entity entity, const T& component)
		{
			ComponentStorage<T>& storage = GetStorage<T>();
			return storage.AddComponent(entity, component);
		}


		template <typename T>
		void ForceStorage()
		{
			getOrCreateStorage<T>();
		}	

		template <typename T>
		void RemoveComponent(Entity entity, const Signature& signature)
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			uint32_t updatedEntity = m_StoragePool.Get<ComponentStorage<T>>(offset)->RemoveComponent(entity);
		}

		template <typename T>
		T& GetComponent(Entity entity)
		{
			size_t offset = IComponent::GetComponentID<T>() * sizeof(ComponentStorage<T>);
			ComponentStorage<T>* storage = static_cast<ComponentStorage<T>*>(m_StoragePool.Get<IComponentStorage>(offset));
			return storage->GetComponent(entity);
		}

		template <typename T>
		const T& GetComponent(Entity entity) const
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

		IComponentStorage* GetIStorage(size_t offset)
		{
			return m_StoragePool.Get<IComponentStorage>(offset);
		}
		const IComponentStorage* GetIStorage(size_t offset) const
		{
			return m_StoragePool.Get<IComponentStorage>(offset);
		}

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const
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
			size_t id = (size_t)IComponent::GetComponentID<T>();
			size_t offset = id * sizeof(ComponentStorage<T>);
			
			if (id >= m_Count)
				resizeStorages(id + 1);		
			if (!m_StorageCreated[id])
			{
				m_StoragePool.Allocate<ComponentStorage<T>>(offset);
				m_StorageCreated[id] = true;
				m_NumberOfStorages++;
			}
			return m_StoragePool.Get<ComponentStorage<T>>(offset);
		}
		void resizeStorages(size_t count);
		void deallocateStorages();
	private:
		Pool m_StoragePool;
		std::vector<bool> m_StorageCreated;
		size_t m_Count;
		size_t m_NumberOfStorages;

		static constexpr size_t sc_InitialStorageCapacity = 10;
		friend class ECSSerializer;
		friend class ECSManager;
	};
	
}