#pragma once
#include "Types.h"
#include "XYZ/Utils/DataStructures/FreeList.h"
#include "XYZ/Core/Core.h"

#include <unordered_map>
#include <array>

namespace XYZ {

	class ComponentManager;
	/*! @class IComponentStorage
	* @brief interface of storage for components
	*/
	class IComponentStorage
	{
		friend class ComponentManager;
	public:
		/**
		* virtual destructor
		*/
		virtual ~IComponentStorage() = default;
		virtual void EntityDestroyed(uint32_t entity) = 0;

	};
	
	

	/*! @class ComponentStorage
	* @brief storage for new Component of components
	*/
	template<typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		ComponentStorage()
			: m_Components(MAX_ENTITIES)
		{}

		virtual ~ComponentStorage()
		{}

		/**
		* Check if contains entity
		* @param[in] entity
		* @return
		*/
		bool Contains(uint32_t entity)
		{
			return m_Lookup.find(entity) != m_Lookup.end();
		}

		/**
		* Add new component to entity
		* @param[in] entity
		* @tparam[in] component
		*/
		T* AddComponent(uint32_t entity,const T& component)
		{
			XYZ_ASSERT(m_Lookup.find(entity) == m_Lookup.end(), "Entity ",entity," already contains component");	
			int32_t index = m_Components.Insert(component);
			m_Lookup[entity] = index;
			return &m_Components[index];
		}

		template<typename T, typename ...Args>
		T* EmplaceComponent(uint32_t entity, Args&&... args)
		{
			XYZ_ASSERT(m_Lookup.find(entity) == m_Lookup.end(), "Entity ", entity, " already contains component");
			int32_t index = m_Components.Emplace(std::forward<Args>(args)...);
			m_Lookup[entity] = index;
			return &m_Components[index];
		}
		/**
		* Remove component from the entity
		* @param[in] entity
		*/
		void RemoveComponent(uint32_t entity)
		{
			XYZ_ASSERT(m_Lookup.find(entity) != m_Lookup.end(), "Removing non-existent component");

			int32_t removeIndex = m_Lookup[entity];
			m_Components.Erase(removeIndex);
			m_Lookup.erase(entity);
		}

		/**
		* @param[in] entity
		* @return index of component in the storage
		*/
		int GetComponentIndex(uint32_t entity)
		{
			XYZ_ASSERT(m_Lookup.find(entity) != m_Lookup.end(), "Retrieving non-existent component.");
			return m_Lookup[entity];
		}

		/**
		* @param[in] entity
		* @return pointer to the component
		*/
		T* GetComponent(uint32_t entity)
		{
			XYZ_ASSERT(m_Lookup.find(entity) != m_Lookup.end(), "Retrieving non-existent component.");
			return &m_Components[m_Lookup[entity]];
		}

		/**
		* Remove entity from storage if entity is destroyed
		* @param[in] entity
		*/
		virtual void EntityDestroyed(uint32_t entity) override
		{
			if (Contains(entity))
				RemoveComponent(entity);
		}

		T& operator [](int32_t index)
		{
			return m_Components[index];
		}
		
		int Size() const
		{
			return m_Components.Range();
		}

	private:
		FreeList<T> m_Components;
		std::unordered_map<uint32_t, int32_t> m_Lookup;
	};

	

}