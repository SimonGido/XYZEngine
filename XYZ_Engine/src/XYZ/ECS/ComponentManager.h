#pragma once
#include "Types.h"
#include "Component.h"
#include "ComponentStorage.h"
#include "ComponentGroup.h"

#include "XYZ/Core/Core.h"

#include <unordered_map>
#include <memory>

namespace XYZ {
	/*! @class ComponentManager
	* @brief manager of component storages
	*/
	class ComponentManager
	{
	public:
		~ComponentManager()
		{
			for (auto group : m_ComponentGroups)
				delete group;
		}

		template <typename ...Types>
		void CreateGroup(ECSManager * ecs)
		{
			IComponentGroup* newGroup = new ComponentGroup<Types...>(ecs);
			for (auto group : m_ComponentGroups)
			{
				XYZ_ASSERT(group->GetSignature() != newGroup->GetSignature(), "Creating same group twice");
			}
			
			m_ComponentGroups.push_back(newGroup);
		}

		template <typename ...Types>
		ComponentGroup<Types...>* GetGroup()
		{
			Signature signature;
			std::initializer_list<uint16_t> componentTypes{ Types::GetID()... };
			for (auto it : componentTypes)
				signature.set(it);

			for (auto group : m_ComponentGroups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature())
				{
					return (ComponentGroup<Types...>*)group;
				}
			}
			XYZ_ASSERT(false, "Accessing non existing group");
			return nullptr;		
		}

		void AddToGroup(uint32_t entity, Signature signature)
		{
			for (auto group : m_ComponentGroups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature())
				{
					group->AddEntity(entity);
					break;
				}
			}
		}

		void RemoveFromGroup(uint32_t entity, Signature signature)
		{
			for (auto group : m_ComponentGroups)
			{
				if ((group->GetSignature() & signature) == group->GetSignature())
				{
					group->RemoveEntity(entity);
					break;
				}
			}
		}

		template<typename T>
		void UnRegisterComponent()
		{
			uint16_t id = IComponent::GetID<T>();
			XYZ_ASSERT(m_Components.find(id) != m_Components.end(), "Unregistering not existing componet Component.");
			m_Components.erase(id);
		}
		/**
		* Add component to the storage
		* @param[in] entity
		* @tparam[in] component
		*/
		template<typename T>
		T* AddComponent(uint32_t entity,const T& component)
		{
			uint16_t id = IComponent::GetID<T>();
			if(m_Components.find(id) == m_Components.end())
				RegisterComponent<T>();
			return GetComponentStorage<T>()->AddComponent(entity, component);
		}


		template<typename T, typename ...Args>
		T* EmplaceComponent(uint32_t entity, Args&&... args)
		{
			uint16_t id = IComponent::GetID<T>();
			if (m_Components.find(id) == m_Components.end())
				RegisterComponent<T>();
			return GetComponentStorage<T>()->EmplaceComponent<T>(entity, std::forward<Args>(args)...);
		}

		/**
		* @return id representation of the component
		*/
		template<typename T>
		ComponentType GetComponentType()
		{
			uint16_t id = IComponent::GetID<T>();
			if (m_Components.find(id) == m_Components.end())
				RegisterComponent<T>();
			return id;
		}

		/**
		* @return storage of the component
		*/
		template<typename T>
		std::shared_ptr<ComponentStorage<T>> GetComponentStorage()
		{
			uint16_t id = IComponent::GetID<T>();
			if (m_Components.find(id) == m_Components.end())
				RegisterComponent<T>();
			return std::static_pointer_cast<ComponentStorage<T>>(m_Components[id]);
		}


		/**
		* @param[in] entity
		* @return reference to the component of entity
		*/
		template<typename T>
		T* GetComponent(uint32_t entity)
		{
			return GetComponentStorage<T>()->GetComponent(entity);
		}

		/**
		* @param[in] entity
		* @return index of the component of the entity
		*/
		template <typename T>
		int GetComponentIndex(uint32_t entity)
		{
			return GetComponentStorage<T>()->GetComponentIndex(entity);
		}

		/**
		* @param[in] entity
		* @return component of the entity
		*/
		template<typename T>
		void RemoveComponent(uint32_t entity)
		{
			auto c = GetComponent(entity);
			GetComponentStorage<T>()->RemoveComponent(entity);
		}

		/**
		* Check if entity contains component
		* @param[in] entity
		* @return
		*/
		template <typename T>
		bool Contains(uint32_t entity)
		{
			return GetComponentStorage<T>()->Contains(entity);
		}

		/**
		* Delete all the components of the entity
		* @param[in] entity
		*/
		void EntityDestroyed(uint32_t entity)
		{
			for (auto const& it : m_Components)
			{
				it.second->EntityDestroyed(entity);
			}
		}

	private:
		/**
		* Register new component storage
		*/
		template<typename T>
		void RegisterComponent()
		{
			uint16_t id = IComponent::GetID<T>();
			XYZ_ASSERT(id < MAX_COMPONENTS, "Registering more than max components");

			std::shared_ptr<IComponentStorage> componentStorage = std::make_shared<ComponentStorage<T> >();
			m_Components.insert({ id,componentStorage });
		}


	private:
		std::unordered_map<uint16_t, std::shared_ptr<IComponentStorage> > m_Components;

		std::vector<IComponentGroup*> m_ComponentGroups;
		
	};

}