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
		Entity CreateEntity();

		/**
		* @param[in] entity
		* @return signature of the entity
		*/
		Signature GetSignature(Entity entity);
		/**
		* Destroy entity
		* @param[in] entity
		*/
		void DestroyEntity(Entity entity);
		/**
		* Set new signature for the entity
		* @param[in] entity
		* @param[in] signature
		*/
		void SetSignature(Entity entity, Signature signature);

	private:
		unsigned int m_EntitiesInUse;

		// Every entity has Signature representing components that it contains
		// System with same signatures will use entity
		FreeList<Signature> m_Signatures;
	};
}