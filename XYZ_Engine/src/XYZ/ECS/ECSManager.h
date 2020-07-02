#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
#include "XYZ/Core/Singleton.h"
#include "XYZ/Event/Event.h"

#include <memory>

namespace XYZ {
	class ECSManager : public Singleton<ECSManager>
	{
	public:
		ECSManager(token);
		
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{	
			return m_SystemManager->RegisterSystem<T>();
		}
		template<typename T>
		void UnRegisterComponent()
		{
			m_ComponentManager->UnRegisterComponent<T>();
		}
		template<typename T>
		T* AddComponent(Entity entity,const T& component)
		{
			auto c = m_ComponentManager->AddComponent<T>(entity, component);

			auto active = m_ComponentManager->GetComponent<ActiveComponent>(entity);
			auto signature = m_EntityManager->GetSignature(entity);
			signature.set(m_ComponentManager->GetComponentType<T>(), 1);
			active->ActiveComponents.set(m_ComponentManager->GetComponentType<T>(), 1);

			m_EntityManager->SetSignature(entity, signature);
			m_SystemManager->EntitySignatureChanged(entity, signature);
			
			return c;
		}
		template<typename T>
		void RemoveComponent(Entity entity)
		{
			m_ComponentManager->RemoveComponent<T>(entity);

			auto active = m_ComponentManager->GetComponent<ActiveComponent>(entity);
			auto signature = m_EntityManager->GetSignature(entity);
			signature.set(m_ComponentManager->GetComponentType<T>(), 0);
			active->activeComponents.set(m_ComponentManager->GetComponentType<T>(), 0);

			m_EntityManager->SetSignature(entity, signature);
			m_SystemManager->EntitySignatureChanged(entity, signature);
		}

		template<typename T>
		void SetSystemSignature(Signature signature)
		{
			m_SystemManager->SetSignature<T>(signature);
		}
		template<typename T>
		ComponentType GetComponentType()
		{
			return m_ComponentManager->GetComponentType<T>();
		}

		template<typename T> 
		std::shared_ptr<ComponentStorage<T>> GetComponentStorage()
		{
			return m_ComponentManager->GetComponentStorage<T>();
		}

		template<typename T>
		T *GetComponent(Entity entity)
		{
			return m_ComponentManager->GetComponent<T>(entity);
		}

		template <typename T>
		int GetComponentIndex(Entity entity)
		{
			return m_ComponentManager->GetComponentIndex<T>(entity);
		}

		template<typename T>
		std::shared_ptr<T> GetSystem()
		{
			return std::static_pointer_cast<T>(m_SystemManager->GetSystem<T>());
		}


		template <typename T>
		bool Contains(Entity entity)
		{
			return m_ComponentManager->Contains<T>(entity);
		}

		void OnEvent(Event& event)
		{
			m_SystemManager->OnEvent(event);
		}

		void DestroyEntity(Entity entity)
		{
			auto signature = GetEntitySignature(entity);
			m_SystemManager->EntityDestroyed(entity, signature);
			m_ComponentManager->EntityDestroyed(entity);
			m_EntityManager->DestroyEntity(entity);
		}
		Signature GetEntitySignature(Entity entity)
		{
			return m_EntityManager->GetSignature(entity);
		}
		Entity CreateEntity()
		{
			Entity entity = m_EntityManager->CreateEntity();
			AddComponent(entity, ActiveComponent{});
			return entity;
		}

	private:
		std::unique_ptr<ComponentManager> m_ComponentManager;
		std::unique_ptr<EntityManager>	  m_EntityManager;
		std::unique_ptr<SystemManager>	  m_SystemManager;
		
	};

}