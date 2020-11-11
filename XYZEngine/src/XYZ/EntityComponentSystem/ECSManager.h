#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"

namespace XYZ {
	namespace ECS {

		class ECSManager
		{
		public:
			uint32_t CreateEntity() { return m_EntityManager.CreateEntity(); };
			void DestroyEntity(uint32_t entity) { m_EntityManager.DestroyEntity(entity); }

			template <typename T>
			T& AddComponent(uint32_t entity, const T& component)
			{
				Signature signature = m_EntityManager.GetSignature(entity);
				m_ComponentManager.AddComponent(entity, signature, component);
				return m_ComponentManager.GetArchetype(signature).GetComponentAs<T>(entity);
			}

			template <typename T>
			void RemoveComponent(uint32_t entity)
			{
				Signature signature = m_EntityManager.GetSignature(entity);
				m_ComponentManager.RemoveComponent(entity, signature);
			}

			template <typename T>
			T& GetComponent(uint32_t entity)
			{
				Signature signature = m_EntityManager.GetSignature(entity);
				return m_ComponentManager.GetArchetype(signature).GetComponent<T>(entity);
			}

			const Signature& GetEntitySignature(uint32_t entity)
			{
				return m_EntityManager.GetSignature(entity);
			}

			void CreateComponentArcheType(const ComponentLayout& layout)
			{
				m_ComponentManager.CreateArcheType(layout);
			}
	
			ComponentArchetype& GetComponentArcheType(const Signature& signature)
			{
				return m_ComponentManager.GetArchetype(signature);
			}
			

		private:
			ComponentManager m_ComponentManager;
			EntityManager m_EntityManager;
		};
	}
}