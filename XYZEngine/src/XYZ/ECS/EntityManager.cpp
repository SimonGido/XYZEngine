#include "stdafx.h"
#include "EntityManager.h"

namespace XYZ {
	EntityManager::EntityManager()
		:
		m_EntitiesInUse(0)
	{
		// Invalid
		m_Signatures.Insert(Signature());
	}
	Entity EntityManager::CreateEntity()
	{
		m_EntitiesInUse++;
		XYZ_ASSERT(m_EntitiesInUse < sc_MaxEntity, "Too many entities in existence.");
		uint32_t entity = m_Signatures.Insert(Signature(0));
		if (m_Valid.size() <= entity)
			m_Valid.resize((size_t)entity + 1);
		m_Valid[entity] = true;
		return entity;		
	}
	Signature& EntityManager::GetSignature(Entity entity)
	{
		XYZ_ASSERT(entity, "Invalid entity");
		return m_Signatures[entity];
	}
	const Signature& EntityManager::GetSignature(Entity entity) const
	{
		XYZ_ASSERT(entity < sc_MaxEntity, "Entity out of range.");
		return m_Signatures[entity];
	}
	void EntityManager::DestroyEntity(Entity entity)
	{
		XYZ_ASSERT(entity, "Invalid entity.");

		// Put the destroyed ID at the back of the queue
		//Restart bitset to zero;
		m_Valid[entity] = false;
		m_Signatures.Erase(entity);
		m_EntitiesInUse--;
	}
	void EntityManager::SetSignature(Entity entity, Signature signature)
	{
		XYZ_ASSERT(entity, "Invalid entity");

		// Put this entity's signature into the array
		m_Signatures[entity] = signature;
	}
}