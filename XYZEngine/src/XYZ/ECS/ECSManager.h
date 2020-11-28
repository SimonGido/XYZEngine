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
			signature.set(HAS_GROUP_BIT, false);
			m_ComponentManager.EntityDestroyed(entity, signature, this);
			m_EntityManager.DestroyEntity(entity); 
		}

		template <typename T>
		T& AddComponent(uint32_t entity, const T& component)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature.test(T::GetComponentID()), "Entity already contains component");
			signature.set(T::GetComponentID(), true);
			auto& result = m_ComponentManager.AddComponent<T>(entity, component);
			m_ComponentManager.AddToGroup(entity, signature, this);
			m_ComponentManager.AddToView(entity, signature);
			return result;
		}
		
		template <typename T>
		bool RemoveComponent(uint32_t entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
			
			if (signature.test(HAS_GROUP_BIT))
			{
				m_ComponentManager.RemoveFromGroup(entity, T::GetComponentID(), signature, this);			
				return true;
			}
			signature.set(T::GetComponentID(), false);
			m_ComponentManager.RemoveComponent<T>(entity, signature);
			return true;
		}

		template <typename T>
		T& GetComponent(uint32_t entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
			if (signature.test(HAS_GROUP_BIT))
			{
				uint8_t* component = nullptr;
				m_ComponentManager.GetFromGroup(entity, signature, T::GetComponentID(), &component);
				if (component)
					return *(T*)component;		
			}
			return m_ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		const T& GetComponent(uint32_t entity) const
		{
			const Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
			if (signature.test(HAS_GROUP_BIT))
			{
				uint8_t* component = nullptr;
				m_ComponentManager.GetFromGroup(entity, signature, T::GetComponentID(), &component);
				if (component)
					return *(T*)component;
			}
			return m_ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		T& GetGroupComponent(uint32_t entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
			XYZ_ASSERT(signature.test(HAS_GROUP_BIT), "Entity does not have group");
			
			uint8_t* component = nullptr;
			m_ComponentManager.GetFromGroup(entity, signature, T::GetComponentID(), &component);
			XYZ_ASSERT(component, "Group does not contain entity");
			return *(T*)component;		
		}

		template <typename T>
		const T& GetGroupComponent(uint32_t entity) const
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
			XYZ_ASSERT(signature.test(HAS_GROUP_BIT), "Entity does not have group");

			uint8_t* component = nullptr;
			m_ComponentManager.GetFromGroup(entity, signature, T::GetComponentID(), &component);
			XYZ_ASSERT(component, "Group does not contain entity");
			return *(T*)component;
		}

		template <typename T>
		T& GetStorageComponent(uint32_t entity)
		{
			XYZ_ASSERT(m_EntityManager.GetSignature(entity).test(T::GetComponentID()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}


		template <typename T>
		const T& GetStorageComponent(uint32_t entity) const
		{
			XYZ_ASSERT(m_EntityManager.GetSignature(entity).test(T::GetComponentID()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}

		const Signature& GetEntitySignature(uint32_t entity) const
		{
			return m_EntityManager.GetSignature(entity);
		}

		bool HasGroup(uint32_t entity) const
		{
			auto& signature = m_EntityManager.GetSignature(entity);
			return signature.test(HAS_GROUP_BIT);
		}

		template <typename T>
		bool Contains(uint32_t entity) const
		{
			auto& signature = m_EntityManager.GetSignature(entity);
			return signature.test(T::GetComponentID());
		}

		template <typename T>
		void ForceStorage()
		{
			m_ComponentManager.ForceStorage<T>();
		}

		template <typename T>
		ComponentStorage<T>* GetStorage()
		{
			return (ComponentStorage<T>*)m_ComponentManager.GetStorage(T::GetComponentID());
		}

		template <typename ...Args>
		ComponentGroup<Args...>& GetGroup()
		{
			Signature signature;
			std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
			for (auto it : componentTypes)
				signature.set(it);

			auto group = m_ComponentManager.GetGroup(signature);
			XYZ_ASSERT(group, "Group does not exist");
			return *(ComponentGroup<Args...>*)group;
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
		ComponentGroup<Args...>& CreateGroup()
		{
			return *m_ComponentManager.CreateGroup<Args...>();
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
			return m_ComponentManager.GetComponentIndex(entity, T::GetComponentID());
		}

		uint32_t GetComponentIndex(uint32_t entity, uint8_t id) const
		{
			return m_ComponentManager.GetComponentIndex(entity, id);
		}

		const uint32_t GetNumberOfEntities() const { return m_EntityManager.GetNumEntities(); }


	private:
		ComponentManager m_ComponentManager;
		EntityManager m_EntityManager;
	};
	
}