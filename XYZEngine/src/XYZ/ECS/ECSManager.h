#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "ComponentView.h"

namespace XYZ {
	 
	class ECSManager
	{
	public:
		ECSManager() = default;
		ECSManager(const ECSManager& other);
		ECSManager(ECSManager&& other) noexcept;

		ECSManager& operator=(const ECSManager& other);
		ECSManager& operator=(ECSManager&& other) noexcept;
		
		Entity CopyEntity(Entity entity);
		Entity CreateEntity();
		void   DestroyEntity(Entity entity);
		void   Clear();

		template <typename T, typename ...Args>
		T& EmplaceComponent(Entity entity, Args&&... args);
		
		template <typename T>
		T& AddComponent(Entity entity, const T& component);
			
		template <typename T>
		bool RemoveComponent(Entity entity);
		
		template <typename T>
		T& GetComponent(Entity entity);
		

		template <typename T>
		const T& GetComponent(Entity entity) const;
		

		template <typename T>
		bool HasComponent(Entity entity) const;
		
		bool HasComponent(Entity entity, uint16_t componentID) const;
		
		bool IsValid(Entity entity) const;
		
		template <typename ...Args>
		void CreateStorage();
		
		template <typename T>
		ComponentStorage<T>&	  GetStorage();
		
		template <typename T>
		const ComponentStorage<T>& GetStorage() const;
		

		IComponentStorage* GetIStorage(uint16_t index);
		const IComponentStorage* GetIStorage(uint16_t index) const;
		

		template <typename ...Args>
		ComponentView<Args...> CreateView();
		

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const;
		
		template <typename T>
		Entity FindEntity(const T& component) const;
		
		
		
		template <typename ComponentType, auto Callable>
		void AddOnConstruction();

		template <typename ComponentType, auto Callable, typename Type>
		void AddOnConstruction(Type* instance);

		template <typename ComponentType, auto Callable>
		void RemoveOnConstruction();

		template <typename ComponentType, auto Callable, typename Type>
		void RemoveOnConstruction(Type* instance);



		template <typename ComponentType, auto Callable>
		void AddOnDestruction();

		template <typename ComponentType, auto Callable, typename Type>
		void AddOnDestruction(Type* instance);

		template <typename ComponentType, auto Callable>
		void RemoveOnDestruction();

		template <typename ComponentType, auto Callable, typename Type>
		void RemoveOnDestruction(Type* instance);


		template <auto Callable>
		void AddOnConstruction();

		template <auto Callable, typename Type>
		void AddOnConstruction(Type* instance);

		template <auto Callable>
		void RemoveOnConstruction();

		template <auto Callable, typename Type>
		void RemoveOnConstruction(Type* instance);



		template <auto Callable>
		void AddOnDestruction();

		template <auto Callable, typename Type>
		void AddOnDestruction(Type* instance);

		template <auto Callable>
		void RemoveOnDestruction();

		template <auto Callable, typename Type>
		void RemoveOnDestruction(Type* instance);



		uint32_t GetEntityVersion(Entity entity) const { return m_EntityManager.GetVersion(entity); }
		uint32_t GetNumberOfEntities() const		   { return m_EntityManager.GetNumEntities(); }

		static uint16_t GetNumberOfRegisteredComponents() { return ComponentManager::GetNextComponentID(); }
	
	private:
		void executeOnConstruction(Entity entity);
		void executeOnDestruction(Entity entity);

		template <typename T>
		void eraseFromVector(std::vector<T>& vec, const T& value);

	private:
		ComponentManager m_ComponentManager;
		EntityManager	 m_EntityManager;
		
		
		std::vector<Delegate<void(Entity)>> m_OnConstruction;
		std::vector<Delegate<void(Entity)>> m_OnDestruction;

		friend class ECSSerializer;
	};
	
	template<typename T, typename ...Args>
	inline T& ECSManager::EmplaceComponent(Entity entity, Args && ...args)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		XYZ_ASSERT(!HasComponent<T>(entity), "Entity already contains component");

		auto& result = m_ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);
		executeOnConstruction(entity);
		return result;
	}

	template<typename T>
	inline T& ECSManager::AddComponent(Entity entity, const T& component)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		XYZ_ASSERT(!HasComponent<T>(entity), "Entity already contains component");

		auto& result = m_ComponentManager.AddComponent<T>(entity, component);
		executeOnConstruction(entity);
		return result;
	}

	template<typename T>
	inline bool ECSManager::RemoveComponent(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		XYZ_ASSERT(HasComponent<T>(entity), "Entity does not have component");

		m_ComponentManager.RemoveComponent<T>(entity);
		executeOnDestruction(entity);
		return true;
	}
	template<typename T>
	inline T& ECSManager::GetComponent(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		XYZ_ASSERT(HasComponent<T>(entity), "Entity does not have component");

		return m_ComponentManager.GetComponent<T>(entity);
	}

	template<typename T>
	inline const T& ECSManager::GetComponent(Entity entity) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		XYZ_ASSERT(HasComponent<T>(entity), "Entity does not have component");

		return m_ComponentManager.GetComponent<T>(entity);
	}
	template<typename T>
	inline bool ECSManager::HasComponent(Entity entity) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		return GetStorage<T>().HasEntity(entity);
	}
	template<typename ...Args>
	inline void ECSManager::CreateStorage()
	{
		(m_ComponentManager.CreateStorage<Args>(), ...);
	}
	template<typename T>
	inline ComponentStorage<T>& ECSManager::GetStorage()
	{
		return m_ComponentManager.GetStorage<T>();
	}
	template<typename T>
	inline const ComponentStorage<T>& ECSManager::GetStorage() const
	{
		return m_ComponentManager.GetStorage<T>();
	}
	template<typename ...Args>
	inline ComponentView<Args...> ECSManager::CreateView()
	{
		(CreateStorage<Args>(), ...);
		return ComponentView<Args...>(m_ComponentManager);
	}
	template<typename T>
	inline uint32_t ECSManager::GetComponentIndex(Entity entity) const
	{
		return m_ComponentManager.GetComponentIndex<T>(entity);
	}
	template<typename T>
	inline Entity ECSManager::FindEntity(const T& component) const
	{
		const auto& entities = m_EntityManager.m_Entities;
		for (int32_t i = 0; i < entities.Range(); ++i)
		{
			if (entities.Valid(i))
			{
				if (HasComponent<T>(static_cast<uint32_t>(i)))
				{
					if (component == m_ComponentManager.GetComponent<T>(i))
						return i;
				}
			}
		}
		return Entity();
	}

	template<typename ComponentType, auto Callable>
	inline void ECSManager::AddOnConstruction()
	{
		GetStorage<ComponentType>().AddOnConstruction<Callable>();
	}

	template<typename ComponentType, auto Callable, typename Type>
	inline void ECSManager::AddOnConstruction(Type* instance)
	{
		GetStorage<ComponentType>().AddOnConstruction<Callable>(instance);
	}

	template<typename ComponentType, auto Callable>
	inline void ECSManager::RemoveOnConstruction()
	{
		GetStorage<ComponentType>().RemoveOnConstruction<Callable>();
	}

	template<typename ComponentType, auto Callable, typename Type>
	inline void ECSManager::RemoveOnConstruction(Type* instance)
	{
		GetStorage<ComponentType>().RemoveOnConstruction<Callable>(instance);
	}

	template<typename ComponentType, auto Callable>
	inline void ECSManager::AddOnDestruction()
	{
		GetStorage<ComponentType>().AddOnDestruction<Callable>();
	}

	template<typename ComponentType, auto Callable, typename Type>
	inline void ECSManager::AddOnDestruction(Type* instance)
	{
		GetStorage<ComponentType>().AddOnDestruction<Callable>(instance);
	}

	template<typename ComponentType, auto Callable>
	inline void ECSManager::RemoveOnDestruction()
	{
		GetStorage<ComponentType>().RemoveOnDestruction<Callable>();
	}

	template<typename ComponentType, auto Callable, typename Type>
	inline void ECSManager::RemoveOnDestruction(Type* instance)
	{
		GetStorage<ComponentType>().RemoveOnDestruction<Callable>(instance);
	}



	template<auto Callable>
	inline void ECSManager::AddOnConstruction()
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>();
		m_OnConstruction.push_back(deleg);
	}

	template<auto Callable, typename Type>
	inline void ECSManager::AddOnConstruction(Type* instance)
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnConstruction.push_back(deleg);
	}

	template<auto Callable>
	inline void ECSManager::RemoveOnConstruction()
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnConstruction, deleg);
	}

	template<auto Callable, typename Type>
	inline void ECSManager::RemoveOnConstruction(Type* instance)
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnConstruction, deleg);
	}

	template<auto Callable>
	inline void ECSManager::AddOnDestruction()
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>();
		m_OnDestruction.push_back(deleg);
	}

	template<auto Callable, typename Type>
	inline void ECSManager::AddOnDestruction(Type* instance)
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnDestruction.push_back(deleg);
	}

	template<auto Callable>
	inline void ECSManager::RemoveOnDestruction()
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnDestruction, deleg);
	}

	template<auto Callable, typename Type>
	inline void ECSManager::RemoveOnDestruction(Type* instance)
	{
		Delegate<void(Entity)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnDestruction, deleg);
	}
	template<typename T>
	inline void ECSManager::eraseFromVector(std::vector<T>& vec, const T& value)
	{
		for (auto it = vec.begin(); it != vec.end();)
		{
			if ((*it) == value)
			{
				it = vec.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}