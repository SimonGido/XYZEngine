#pragma once
#include "Types.h"
#include "XYZ/Utils/DataStructures/FreeList.h"

#include <queue>
#include <array>

namespace XYZ {
	/*! @class EntityManager
	* @brief Manager for entities
	*/
	class EntityManager
	{
	public:
		/**
		* Constructor
		*/
		EntityManager();

		/**
		* Creates new entity
		* @return new entity
		*/
		uint32_t CreateEntity();

		/**
		* @param[in] entity
		* @return signature of the entity
		*/
		Signature GetSignature(uint32_t entity);
		/**
		* Destroy entity
		* @param[in] entity
		*/
		void DestroyEntity(uint32_t entity);
		/**
		* Set new signature for the entity
		* @param[in] entity
		* @param[in] signature
		*/
		void SetSignature(uint32_t entity, Signature signature);

		uint32_t GetNumEntitiesInUse() const { return m_EntitiesInUse; }
	private:
		uint32_t m_EntitiesInUse;

		// Every entity has Signature representing components that it contains
		// System with same signatures will use entity
		FreeList<Signature> m_Signatures;
	};
}