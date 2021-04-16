#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "CallbackManager.h"
#include "ComponentView.h"

namespace XYZ {
	 
	class ECSManager
	{
	public:
		ECSManager() = default;
		ECSManager(const ECSManager& other);
		ECSManager(ECSManager&& other) noexcept;

		ECSManager& operator=(ECSManager&& other) noexcept;

		Entity CreateEntity();
		void DestroyEntity(Entity entity);
		void Clear();

		template <typename T>
		void AddListener(const std::function<void(uint32_t, CallbackType)>& callback, void* instance)
		{
			m_CallbackManager.AddListener<T>(callback, instance);
		}

		template <typename T>
		void RemoveListener(void* instance)
		{
			m_CallbackManager.RemoveListener<T>(instance);
		}

		template <typename T, typename ...Args>
		T& EmplaceComponent(Entity entity, Args&&... args)
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			m_ComponentManager.ForceStorage<T>();
			m_EntityManager.SetNumberOfComponents(m_ComponentManager.m_Count);
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature[IComponent::GetComponentID<T>()], "Entity already contains component");
			signature.Set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);
			m_CallbackManager.OnComponentCreate<T>(entity);
			return result;
		}
		template <typename T>
		T& AddComponent(Entity entity, const T& component)
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			m_ComponentManager.ForceStorage<T>();
			m_EntityManager.SetNumberOfComponents(m_ComponentManager.m_Count);
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature[IComponent::GetComponentID<T>()], "Entity already contains component");
			signature.Set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.AddComponent<T>(entity, component);
			m_CallbackManager.OnComponentCreate<T>(entity);
			return result;
		}
		
		template <typename T>
		bool RemoveComponent(Entity entity)
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature[IComponent::GetComponentID<T>()], "Entity does not have component");
			
			signature.set(IComponent::GetComponentID<T>(), false);
			m_CallbackManager.OnComponentRemove<T>(entity);
			m_ComponentManager.RemoveComponent<T>(entity, signature);

			return true;
		}

		template <typename T>
		T& GetComponent(Entity entity)
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature[IComponent::GetComponentID<T>()], "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		const T& GetComponent(Entity entity) const
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			const Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature[IComponent::GetComponentID<T>()], "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}


		const Signature& GetEntitySignature(Entity entity) const
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			return m_EntityManager.GetSignature(entity);
		}

		template <typename T>
		bool Contains(Entity entity) const
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			auto& signature = m_EntityManager.GetSignature(entity);
			if (signature.Size() <= IComponent::GetComponentID<T>())
				return false;
			return signature[IComponent::GetComponentID<T>()];
		}
	
		bool Contains(Entity entity, uint16_t componentID) const
		{
			XYZ_ASSERT(IsValid(entity), "Entity is invalid");
			auto& signature = m_EntityManager.GetSignature(entity);
			if (signature.Size() <= componentID)
				return false;
			return signature[componentID];
		}

		bool IsValid(Entity entity) const
		{
			return m_EntityManager.m_Valid.size() > (uint32_t)entity && m_EntityManager.m_Valid[(uint32_t)entity];
		}

		template <typename ...Args>
		void ForceStorage()
		{
			(m_ComponentManager.ForceStorage<Args>(), ...);
			m_EntityManager.SetNumberOfComponents(m_ComponentManager.m_Count);
		}

		template <typename T>
		ComponentStorage<T>& GetStorage()
		{
			return m_ComponentManager.GetStorage<T>();
		}

		template <typename T>
		const ComponentStorage<T>& GetStorage() const
		{
			return m_ComponentManager.GetStorage<T>();
		}

		IComponentStorage& GetIStorage(uint16_t index)
		{
			size_t offset = (size_t)index * sizeof(ComponentStorage<IComponent>);
			return *m_ComponentManager.GetIStorage(offset);
		}

		const IComponentStorage& GetIStorage(uint16_t index) const
		{
			size_t offset = (size_t)index * sizeof(ComponentStorage<IComponent>);
			return *m_ComponentManager.GetIStorage(offset);
		}

		template <typename ...Args>
		ComponentView<Args...> CreateView()
		{
			return ComponentView<Args...>(m_ComponentManager);
		}

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const
		{
			return m_ComponentManager.GetComponentIndex<T>(entity);
		}

		template <typename T>
		Entity FindEntity(const T& component) const
		{
			for (int32_t i = 0; i < m_EntityManager.m_Bitset.GetNumberOfSignatures(); ++i)
			{
				if (m_EntityManager.m_Bitset[i][IComponent::GetComponentID<T>()])
				{
					if (component == m_ComponentManager.GetComponent<T>(i))
						return i;
				}
			}
			return Entity();
		}

		const uint32_t GetNumberOfEntities() const { return m_EntityManager.GetNumEntities(); }

		size_t GetNumberOfRegisteredComponentTypes() const { return m_ComponentManager.GetNumberOfRegisteredStorages(); }
	private:
		ComponentManager m_ComponentManager;
		CallbackManager m_CallbackManager;
		EntityManager m_EntityManager;
	
		friend class ECSSerializer;
	};
	
}