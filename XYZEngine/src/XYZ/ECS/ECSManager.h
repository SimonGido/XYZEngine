#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"

namespace XYZ {
	 
	class ECSManager
	{
	public:
		uint32_t CreateEntity() { return m_EntityManager.CreateEntity(); };

		void DestroyEntity(uint32_t entity) 
		{ 
			auto& signature = m_EntityManager.GetSignature(entity);
			m_ComponentManager.EntityDestroyed(entity, signature);
			m_EntityManager.DestroyEntity(entity); 
		}

		template <typename T, typename ...Args>
		T& EmplaceComponent(uint32_t entity, Args&&... args)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature.test(IComponent::GetComponentID<T>()), "Entity already contains component");
			signature.set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);
			m_ComponentManager.AddToView(entity, signature);
			return result;
		}
		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature.test(IComponent::GetComponentID<T>()), "Entity already contains component");
			signature.set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.AddComponent<T>(entity, component);
			m_ComponentManager.AddToView(entity, signature);
			return result;
		}
		
		template <typename T>
		bool RemoveComponent(uint32_t entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			
			signature.set(IComponent::GetComponentID<T>(), false);
			m_ComponentManager.RemoveComponent<T>(entity, signature);
			return true;
		}

		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		const T& GetComponent(uint32_t entity) const
		{
			const Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}


		const Signature& GetEntitySignature(uint32_t entity) const
		{
			return m_EntityManager.GetSignature(entity);
		}

		template <typename T>
		bool Contains(uint32_t entity) const
		{
			auto& signature = m_EntityManager.GetSignature(entity);
			return signature.test(IComponent::GetComponentID<T>());
		}

		bool IsValid(uint32_t entity) const
		{
			return m_EntityManager.m_Valid.size() > entity && m_EntityManager.m_Valid[entity];
		}

		template <typename T>
		void ForceStorage()
		{
			m_ComponentManager.ForceStorage<T>();
		}

		template <typename T>
		ComponentStorage& GetStorage()
		{
			return m_ComponentManager.GetStorage<T>();
		}

		template <typename T>
		const ComponentStorage& GetStorage() const
		{
			return m_ComponentManager.GetStorage<T>();
		}

		template <typename ...Args>
		ComponentView<Args...>& GetView()
		{
			Signature signature;
			std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
			for (auto it : componentTypes)
				signature.set(it);

			auto view = m_ComponentManager.GetView(signature);
			
			XYZ_ASSERT(view, "View does not exist");
			return *(ComponentView<Args...>*)view;
		}

		template <typename ...Args>
		ComponentView<Args...>& CreateView()
		{
			auto view = m_ComponentManager.CreateView<Args...>(this);
			for (uint32_t i = 0; i < m_EntityManager.m_Signatures.Range(); ++i)
			{
				const Signature& signature = m_EntityManager.m_Signatures[i];
				if ((view->GetSignature() & signature) == view->GetSignature())
					view->AddEntity(i);
			}
			return *view;
		}

		template <typename T>
		uint32_t GetComponentIndex(uint32_t entity) const
		{
			return m_ComponentManager.GetComponentIndex(entity, IComponent::GetComponentID<T>());
		}

		uint32_t GetComponentIndex(uint32_t entity, uint8_t id) const
		{
			return m_ComponentManager.GetComponentIndex(entity, id);
		}

		template <typename T>
		uint32_t FindEntity(const T& component) const
		{
			for (int32_t i = 0; i < m_EntityManager.m_Signatures.Range();++i)
			{
				if (m_EntityManager.m_Signatures[i].test(IComponent::GetComponentID<T>()))
				{
					if (component == m_ComponentManager.GetComponent<T>(i))
						return i;
				}
			}
			return NULL_ENTITY;
		}

		const uint32_t GetNumberOfEntities() const { return m_EntityManager.GetNumEntities(); }

		size_t GetNumberOfRegisteredComponentTypes() const { return m_ComponentManager.GetNumberOfStorages(); }
	private:
		ComponentManager m_ComponentManager;
		EntityManager m_EntityManager;
	};
	
}