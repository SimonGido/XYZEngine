#include "stdafx.h"
#include "EntityManager.h"

namespace XYZ {
	EntityManager::EntityManager()
		: m_EntitiesInUse(0)
	{

	}
	uint32_t EntityManager::CreateEntity()
	{
		XYZ_ASSERT(m_EntitiesInUse < MAX_ENTITIES, "Too many entities in existence.");

		// Take an ID from the front of the queue
		m_EntitiesInUse++;
		uint32_t entity = (uint32_t)m_Signatures.Insert({ Signature(),0 });
		m_Signatures[(int)entity].Entity = entity;

		return entity;
	}
	Signature EntityManager::GetSignature(uint32_t entity)
	{
		XYZ_ASSERT(entity < MAX_ENTITIES, "Entity out of range.");
		return m_Signatures[entity].Signature;
	}
	void EntityManager::DestroyEntity(uint32_t entity)
	{
		XYZ_ASSERT(entity < MAX_ENTITIES, "Entity out of range.");

		// Put the destroyed ID at the back of the queue
		//Restart bitset to zero;
		m_Signatures[entity].Signature.reset();
		m_Signatures.Erase(entity);
		m_EntitiesInUse--;
	}
	void EntityManager::SetSignature(uint32_t entity, Signature signature)
	{
		XYZ_ASSERT(entity < MAX_ENTITIES, "Entity out of range.");

		// Put this entity's signature into the array
		m_Signatures[entity].Signature = signature;
	}

}