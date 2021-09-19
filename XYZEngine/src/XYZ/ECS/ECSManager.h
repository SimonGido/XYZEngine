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
		
		const Signature& GetEntitySignature(Entity entity) const;
		
		template <typename T>
		bool Contains(Entity entity) const;
		
		bool Contains(Entity entity, uint16_t componentID) const;
		
		bool IsValid(Entity entity) const;
		
		template <typename ...Args>
		void CreateStorage();
		
		template <typename T>
		ComponentStorage<T>& GetStorage();
		
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
		
		
		
		template <typename FuncT, typename ...Args>
		void	 AddOnConstruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnConstruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnConstruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnConstruction(void(Type::* func)(), Type* instance);


		template <typename FuncT, typename ...Args>
		void	 AddOnDestruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnDestruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnDestruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnDestruction(void(Type::* func)(), Type* instance);




		template <typename T, typename FuncT, typename ...Args>
		void	 AddOnComponentConstruction(FuncT&& func, Args&& ...args);
		
		template <typename T, typename Type>
		void	 AddOnComponentConstruction(void(Type::* func)(), Type* instance);
	
		template <typename T, typename FuncT>
		void	 RemoveOnComponentConstruction(FuncT&& func);

		template <typename T, typename Type>
		void	 RemoveOnComponentConstruction(void(Type::* func)(), Type* instance);


		template <typename T, typename FuncT, typename ...Args>
		void	 AddOnComponentDestruction(FuncT&& func, Args&& ...args);

		template <typename T, typename Type>
		void	 AddOnComponentDestruction(void(Type::* func)(), Type* instance);

		template <typename T, typename FuncT>
		void	 RemoveOnComponentDestruction(FuncT&& func);

		template <typename T, typename Type>
		void	 RemoveOnComponentDestruction(void(Type::* func)(), Type* instance);



		uint32_t GetEntityVersion(Entity entity) const { return m_EntityManager.GetVersion(entity); }
		uint32_t GetNumberOfEntities() const		   { return m_EntityManager.GetNumEntities(); }
		uint32_t GetHighestID() const				   { return (uint32_t)m_EntityManager.m_Versions.size(); }

		uint16_t		GetNumberOfCreatedStorages() const { return m_ComponentManager.GetNumberOfCreatedStorages(); }
		static uint16_t GetNumberOfRegisteredComponents() { return ComponentManager::s_NextComponentTypeID; }
	
	private:
		void executeOnConstruction();
		void executeOnDestruction();
	private:
		ComponentManager m_ComponentManager;
		EntityManager	 m_EntityManager;
		
		struct Callback
		{
			Callback() = default;
			Callback(const std::function<void()>& callable, void* funcPtr, void* instance)
				:
				Callable(callable),
				FunctionPointer(funcPtr),
				Instance(instance)
			{
			}
			std::function<void()> Callable;
			void* FunctionPointer = nullptr;
			void* Instance = nullptr;
		};

		std::vector<Callback> m_OnConstruction;
		std::vector<Callback> m_OnDestruction;

		friend class ECSSerializer;
	};
	
	template<typename T, typename ...Args>
	inline T& ECSManager::EmplaceComponent(Entity entity, Args && ...args)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		// Make sure storage for component exists
		m_ComponentManager.CreateStorage<T>();
		// Update bitsets
		m_EntityManager.SetNumberOfComponents(ComponentManager::s_NextComponentTypeID);
		// Update signature
		Signature& signature = m_EntityManager.GetSignature(entity);
		XYZ_ASSERT(!signature[Component<T>::ID()], "Entity already contains component");
		signature.Set(Component<T>::ID(), true);
		auto& result = m_ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);
		executeOnConstruction();
		return result;
	}

	template<typename T>
	inline T& ECSManager::AddComponent(Entity entity, const T& component)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		// Make sure storage for component exists
		m_ComponentManager.CreateStorage<T>();
		// Update bitsets
		m_EntityManager.SetNumberOfComponents(ComponentManager::s_NextComponentTypeID);
		// Update signature
		Signature& signature = m_EntityManager.GetSignature(entity);
		XYZ_ASSERT(!signature[Component<T>::ID()], "Entity already contains component");
		signature.Set(Component<T>::ID(), true);
		auto& result = m_ComponentManager.AddComponent<T>(entity, component);
		executeOnConstruction();
		return result;
	}

	template<typename T>
	inline bool ECSManager::RemoveComponent(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		Signature& signature = m_EntityManager.GetSignature(entity);
		XYZ_ASSERT(signature[Component<T>::ID()], "Entity does not have component");

		signature.Set(Component<T>::ID(), false);
		m_ComponentManager.RemoveComponent<T>(entity, signature);
		executeOnDestruction();
		return true;
	}
	template<typename T>
	inline T& ECSManager::GetComponent(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		Signature& signature = m_EntityManager.GetSignature(entity);
		XYZ_ASSERT(signature[Component<T>::ID()], "Entity does not have component");
		return m_ComponentManager.GetComponent<T>(entity);
	}

	template<typename T>
	inline const T& ECSManager::GetComponent(Entity entity) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		const Signature& signature = m_EntityManager.GetSignature(entity);
		XYZ_ASSERT(signature[Component<T>::ID()], "Entity does not have component");
		return m_ComponentManager.GetComponent<T>(entity);
	}
	template<typename T>
	inline bool ECSManager::Contains(Entity entity) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		if (!Component<T>::Registered())
			return false;
		auto& signature = m_EntityManager.GetSignature(entity);
		if (signature.Size() <= Component<T>::ID())
			return false;
		return signature[Component<T>::ID()];
	}
	template<typename ...Args>
	inline void ECSManager::CreateStorage()
	{
		(m_ComponentManager.CreateStorage<Args>(), ...);
		m_EntityManager.SetNumberOfComponents(ComponentManager::s_NextComponentTypeID);
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
		for (int32_t i = 0; i < m_EntityManager.m_Bitset.GetNumberOfSignatures(); ++i)
		{
			if (m_EntityManager.m_Bitset[i][Component<T>::ID()])
			{
				if (component == m_ComponentManager.GetComponent<T>(i))
					return i;
			}
		}
		return Entity();
	}

	template<typename FuncT, typename ...Args>
	inline void ECSManager::AddOnConstruction(FuncT&& func, Args && ...args)
	{
		m_OnConstruction.emplace_back([=]() {
			func((args)...);
		}, func, nullptr);
	}

	template<typename Type>
	inline void ECSManager::AddOnConstruction(void(Type::* func)(), Type* instance)
	{
		m_OnConstruction.emplace_back([=]() {
			(instance->*func)();
		}, &func, instance);
	}

	template<typename FuncT>
	inline void ECSManager::RemoveOnConstruction(FuncT&& func)
	{
		for (size_t i = 0; i < m_OnConstruction.size(); ++i)
		{
			if (m_OnConstruction[i].FunctionPointer == func)
			{
				m_OnConstruction.erase(m_OnConstruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void ECSManager::RemoveOnConstruction(void(Type::* func)(), Type* instance)
	{
		for (size_t i = 0; i < m_OnConstruction.size(); ++i)
		{
			if (m_OnConstruction[i].FunctionPointer == &func && m_OnConstruction[i].Instance == instance)
			{
				m_OnConstruction.erase(m_OnConstruction.begin() + i);
				return;
			}
		}
	}

	template<typename FuncT, typename ...Args>
	inline void ECSManager::AddOnDestruction(FuncT&& func, Args && ...args)
	{
		m_OnDestruction.emplace_back([=]() {
			func((args)...);
		}, func, nullptr);
	}

	template<typename Type>
	inline void ECSManager::AddOnDestruction(void(Type::* func)(), Type* instance)
	{
		m_OnDestruction.emplace_back([=]() {
			(instance->*func)();
		}, &func, instance);
	}

	template<typename FuncT>
	inline void ECSManager::RemoveOnDestruction(FuncT&& func)
	{
		for (size_t i = 0; i < m_OnDestruction.size(); ++i)
		{
			if (m_OnDestruction[i].FunctionPointer == func)
			{
				m_OnDestruction.erase(m_OnDestruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void ECSManager::RemoveOnDestruction(void(Type::* func)(), Type* instance)
	{
		for (size_t i = 0; i < m_OnDestruction.size(); ++i)
		{
			if (m_OnDestruction[i].FunctionPointer == &func && m_OnDestruction[i].Instance == instance)
			{
				m_OnDestruction.erase(m_OnDestruction.begin() + i);
				return;
			}
		}
	}

	template<typename T, typename FuncT, typename ...Args>
	inline void ECSManager::AddOnComponentConstruction(FuncT&& func, Args && ...args)
	{
		GetStorage<T>().AddOnConstruction(func, std::forward<Args>(args)...);
	}

	template<typename T, typename Type>
	inline void ECSManager::AddOnComponentConstruction(void(Type::* func)(), Type* instance)
	{
		GetStorage<T>().AddOnConstruction(func, instance);
	}

	template<typename T, typename FuncT>
	inline void ECSManager::RemoveOnComponentConstruction(FuncT&& func)
	{
		GetStorage<T>().RemoveOnConstruction(func);
	}

	template<typename T, typename Type>
	inline void ECSManager::RemoveOnComponentConstruction(void(Type::* func)(), Type* instance)
	{
		GetStorage<T>().RemoveOnConstruction(func, instance);
	}

	template<typename T, typename FuncT, typename ...Args>
	inline void ECSManager::AddOnComponentDestruction(FuncT&& func, Args && ...args)
	{
		GetStorage<T>().AddOnDestruction(func, std::forward<Args>(args)...);
	}

	template<typename T, typename Type>
	inline void ECSManager::AddOnComponentDestruction(void(Type::* func)(), Type* instance)
	{
		GetStorage<T>().AddOnDestruction(func, instance);
	}

	template<typename T, typename FuncT>
	inline void ECSManager::RemoveOnComponentDestruction(FuncT&& func)
	{
		GetStorage<T>().RemoveOnDestruction(func);
	}

	template<typename T, typename Type>
	inline void ECSManager::RemoveOnComponentDestruction(void(Type::* func)(), Type* instance)
	{
		GetStorage<T>().RemoveOnDestruction(func, instance);
	}

}