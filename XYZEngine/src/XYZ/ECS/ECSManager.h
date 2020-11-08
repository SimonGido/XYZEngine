#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
#include "ComponentGroup.h"

#include "XYZ/Core/Singleton.h"
#include "XYZ/Event/Event.h"

#include <memory>

namespace XYZ {

	struct ECSCoreData
	{
		ComponentManager ComponentManager;
		EntityManager    EntityManager;
		SystemManager	 SystemManager;
	};

	class ECSManager
	{
	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{	
			return s_Data.SystemManager.RegisterSystem<T>();
		}
		template<typename T>
		void UnRegisterComponent()
		{
			s_Data.ComponentManager.UnRegisterComponent<T>();
		}
		template<typename T>
		T* AddComponent(uint32_t entity,const T& component)
		{
			static_assert(std::is_base_of<Type<T>, T>::value, "Trying to add component that has no type");
			auto c = s_Data.ComponentManager.AddComponent<T>(entity, component);

			auto active = s_Data.ComponentManager.GetComponent<ActiveComponent>(entity);
			auto signature = s_Data.EntityManager.GetSignature(entity);

			// Remove from old group
			s_Data.ComponentManager.RemoveFromGroup(entity, signature);

			signature.set(s_Data.ComponentManager.GetComponentType<T>(), 1);
			active->ActiveComponents.set(s_Data.ComponentManager.GetComponentType<T>(), 1);

			s_Data.EntityManager.SetSignature(entity, signature);
			s_Data.SystemManager.EntitySignatureChanged(entity, signature);

			// Add to new group
			s_Data.ComponentManager.AddToGroup(entity, signature);

			return c;
		}

		template <typename T,typename ...Args>
		T* EmplaceComponent(uint32_t entity, Args&& ... args)
		{
			static_assert(std::is_base_of<Type<T>, T>::value, "Trying to add component that has no type");
			auto c = s_Data.ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);

			auto active = s_Data.ComponentManager.GetComponent<ActiveComponent>(entity);
			auto signature = s_Data.EntityManager.GetSignature(entity);

			// Remove from old group
			s_Data.ComponentManager.RemoveFromGroup(entity, signature);

			signature.set(s_Data.ComponentManager.GetComponentType<T>(), 1);
			active->ActiveComponents.set(s_Data.ComponentManager.GetComponentType<T>(), 1);

			s_Data.EntityManager.SetSignature(entity, signature);
			s_Data.SystemManager.EntitySignatureChanged(entity, signature);

			// Add to new group
			s_Data.ComponentManager.AddToGroup(entity, signature);

			return c;
		}

		template<typename T>
		void RemoveComponent(uint32_t entity)
		{
			s_Data.ComponentManager.RemoveComponent<T>(entity);

			auto active = s_Data.ComponentManager.GetComponent<ActiveComponent>(entity);
			auto signature = s_Data.EntityManager.GetSignature(entity);

			// Remove from old group
			s_Data.ComponentManager.RemoveFromGroup(entity, signature);

			signature.set(s_Data.ComponentManager.GetComponentComponent<T>(), 0);
			active->activeComponents.set(s_Data.ComponentManager.GetComponentType<T>(), 0);

			s_Data.EntityManager.SetSignature(entity, signature);
			s_Data.SystemManager.EntitySignatureChanged(entity, signature);

			// Add to new group
			s_Data.ComponentManager.AddToGroup(entity, signature);
		}

		template<typename T>
		void SetSystemSignature(Signature signature)
		{
			s_Data.SystemManager.SetSignature<T>(signature);
		}
		template<typename T>
		ComponentType GetComponentType()
		{
			return s_Data.ComponentManager.GetComponentType<T>();
		}

		template<typename T> 
		std::shared_ptr<ComponentStorage<T>> GetComponentStorage()
		{
			return s_Data.ComponentManager.GetComponentStorage<T>();
		}

		template<typename T>
		T *GetComponent(uint32_t entity)
		{
			return s_Data.ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		int32_t GetComponentIndex(uint32_t entity)
		{
			return s_Data.ComponentManager.GetComponentIndex<T>(entity);
		}

		template<typename T>
		std::shared_ptr<T> GetSystem()
		{
			return std::static_pointer_cast<T>(s_Data.SystemManager.GetSystem<T>());
		}


		template <typename T>
		bool Contains(uint32_t entity)
		{
			return s_Data.ComponentManager.Contains<T>(entity);
		}
		void DestroyEntity(uint32_t entity)
		{
			auto signature = GetEntitySignature(entity);
			s_Data.SystemManager.EntityDestroyed(entity, signature);
			// Remove from group
			s_Data.ComponentManager.RemoveFromGroup(entity, signature);
			s_Data.ComponentManager.EntityDestroyed(entity);
			s_Data.EntityManager.DestroyEntity(entity);
		}
		Signature GetEntitySignature(uint32_t entity)
		{
			return s_Data.EntityManager.GetSignature(entity);
		}
		uint32_t CreateEntity()
		{
			uint32_t entity = s_Data.EntityManager.CreateEntity();
			EmplaceComponent<ActiveComponent>(entity);
			return entity;
		}

		template <typename ...Types>
		ComponentGroup<Types...>* CreateGroup()
		{
			auto group = s_Data.ComponentManager.CreateGroup<Types...>(this);
			// Add old entities to group
			for (int i = 0; i < s_Data.EntityManager.m_Signatures.Range(); ++i)
			{
				Signature signature = s_Data.EntityManager.m_Signatures[i].Signature;
				uint32_t entity = s_Data.EntityManager.m_Signatures[i].Entity;
				if ((group->GetSignature() & signature) == group->GetSignature())
					group->AddEntity(entity);
			}
			return group;
		}

		template <typename ...Types>
		ComponentGroup<Types...>* GetGroup()
		{
			return s_Data.ComponentManager.GetGroup<Types...>();
		}

	private:
		ECSCoreData s_Data;
		
	};

}