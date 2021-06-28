#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"
#include "Entity.h"
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
		void CreateStorage()
		{
			registerComponentType<T>();
			createStorage<T>();
		}	

		template <typename T>
		void RemoveComponent(Entity entity, const Signature& signature)
		{
			ComponentStorage<T>& storage = GetStorage<T>();
			storage.RemoveComponent(entity);
		}

		template <typename T>
		T& GetComponent(Entity entity)
		{
			ComponentStorage<T>& storage = GetStorage<T>();
			return storage.GetComponent(entity);
		}

		template <typename T>
		const T& GetComponent(Entity entity) const
		{
			const ComponentStorage<T>& storage = GetStorage<T>();
			return storage.GetComponent(entity);
		}

		template <typename T>
		ComponentStorage<T>& GetStorage()
		{
			XYZ_ASSERT(m_Storages[(size_t)Component<T>::ID()], "Storage is not initialized");
			return *static_cast<ComponentStorage<T>*>(m_Storages[(size_t)Component<T>::ID()]);
		}

		template <typename T>
		const ComponentStorage<T>& GetStorage() const
		{
			XYZ_ASSERT(m_Storages[(size_t)Component<T>::ID()], "Storage is not initialized");
			return *static_cast<ComponentStorage<T>*>(m_Storages[(size_t)Component<T>::ID()]);
		}

		IComponentStorage* GetIStorage(uint16_t index)
		{
			return m_Storages[(size_t)index];
		}
		const IComponentStorage* GetIStorage(int16_t index) const
		{
			return m_Storages[(size_t)index];
		}

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const
		{
			const ComponentStorage<T>& storage = GetStorage<T>();
			return storage.GetComponentIndex(entity);
		}

		uint16_t GetNumberOfCreatedStorages() const { return m_StoragesCreated; }

	private:
		template <typename T>
		void createStorage()
		{
			size_t oldSize = m_Storages.size();
			size_t id = (size_t)Component<T>::ID();
			if (oldSize <= id)
				m_Storages.resize(id + 1);		
			for (size_t i = oldSize; i < m_Storages.size(); ++i)
				m_Storages[i] = nullptr;		
			if (m_Storages[id]) // Storage already exists
				return;

			m_Storages[id] = new ComponentStorage<T>();
			m_StoragesCreated++;
		}

		template <typename T>
		static void registerComponentType()
		{
			if (!Component<T>::Registered())
				Component<T>::s_ID = s_NextComponentTypeID++;
		}

		void destroyStorages();

	private:
		std::vector<IComponentStorage*> m_Storages;
		uint16_t						m_StoragesCreated;

		static uint16_t				    s_NextComponentTypeID;

		friend class ECSSerializer;
		friend class ECSManager;
	};
	
}