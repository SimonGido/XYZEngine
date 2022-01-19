#pragma once
#include "ComponentStorage.h"
#include "Component.h"
#include "Types.h"
#include "Entity.h"


namespace XYZ {

	class ComponentManager
	{
	public:
		ComponentManager();	
		ComponentManager(const ComponentManager& other);
		ComponentManager(ComponentManager&& other) noexcept;
		~ComponentManager();

		ComponentManager& operator=(ComponentManager&& other) noexcept;

		void EntityDestroyed(Entity entity);	
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
		void CreateStorage() const
		{
			registerComponentType<T>();
			createStorage<T>();
		}	

		template <typename T>
		void RemoveComponent(Entity entity)
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
			CreateStorage<T>();
			return *static_cast<ComponentStorage<T>*>(m_Storages[static_cast<size_t>(Component<T>::ID())]);
		}

		template <typename T>
		const ComponentStorage<T>& GetStorage() const
		{
			CreateStorage<T>();
			return *static_cast<ComponentStorage<T>*>(m_Storages[static_cast<size_t>(Component<T>::ID())]);
		}

		IComponentStorage* GetIStorage(uint16_t index)
		{
			if ((size_t)index >= m_Storages.size())
				return nullptr;
			return m_Storages[(size_t)index];
		}
		const IComponentStorage* GetIStorage(int16_t index) const
		{
			if ((size_t)index >= m_Storages.size())
				return nullptr;
			return m_Storages[(size_t)index];
		}

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const
		{
			const ComponentStorage<T>& storage = GetStorage<T>();
			return storage.GetComponentIndex(entity);
		}

		const std::vector<IComponentStorage*>& GetIStorages() const { return m_Storages; }

		static uint16_t GetNextComponentID() { return s_NextComponentTypeID; }
	private:
		template <typename T>
		void createStorage() const
		{
			const size_t oldSize = m_Storages.size();
			const size_t id = (size_t)Component<T>::ID();
			
			if (oldSize <= id)
				m_Storages.resize(id + 1);		
			
			if (m_Storages[id]) // Storage already exists
				return;

			for (size_t i = oldSize; i < m_Storages.size(); ++i)
				m_Storages[i] = nullptr;		
			
			m_Storages[id] = new ComponentStorage<T>();
		}

		template <typename T>
		static void registerComponentType()
		{
			if (!Component<T>::Registered())
				Component<T>::s_ID = s_NextComponentTypeID++;
		}

		void destroyStorages();

	private:
		mutable std::vector<IComponentStorage*> m_Storages;

		static uint16_t							s_NextComponentTypeID;
	};
	
}