#include "stdafx.h"
#include "EntityManager.h"

namespace XYZ {
	EntityManager::EntityManager()
		:
		m_EntitiesInUse(0)
	{
		// Invalid
		m_Bitset.CreateSignature();
	}
	EntityManager::EntityManager(const EntityManager& other)
		:
		m_Bitset(other.m_Bitset),
		m_Versions(other.m_Versions),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager::EntityManager(EntityManager&& other) noexcept
		:
		m_Bitset(std::move(other.m_Bitset)),
		m_Versions(std::move(other.m_Versions)),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager& EntityManager::operator=(EntityManager&& other) noexcept
	{
		m_Bitset = std::move(other.m_Bitset);
		m_Versions = std::move(other.m_Versions);
		m_EntitiesInUse = other.m_EntitiesInUse;
		return *this;
	}
	Entity EntityManager::CreateEntity()
	{
		m_EntitiesInUse++;
		XYZ_ASSERT(m_EntitiesInUse < sc_MaxEntity, "Too many entities in existence.");
		const uint32_t entity = (uint32_t)m_Bitset.CreateSignature();
		if (m_Versions.size() <= entity)
		{
			m_Versions.resize(entity + 1);
			m_Versions[entity] = 0;
		}
		else
		{
			m_Versions[entity]++;
		}
		return entity;		
	}
	uint32_t EntityManager::GetVersion(Entity entity) const
	{
		return m_Versions[entity];
	}
	Signature& EntityManager::GetSignature(Entity entity)
	{
		XYZ_ASSERT(entity, "Invalid entity");
		return m_Bitset.GetSignature((int32_t)entity);
	}
	const Signature& EntityManager::GetSignature(Entity entity) const
	{
		XYZ_ASSERT(entity < sc_MaxEntity, "Entity out of range.");
		return m_Bitset.GetSignature((int32_t)entity);
	}
	void EntityManager::DestroyEntity(Entity entity)
	{
		XYZ_ASSERT(entity, "Invalid entity.");

		m_Bitset.DestroySignature(entity);
		m_EntitiesInUse--;
	}
	void EntityManager::SetNumberOfComponents(uint16_t number)
	{
		m_Bitset.SetNumberBits(number);
	}
	void EntityManager::SetSignature(Entity entity, Signature signature)
	{
		XYZ_ASSERT(entity, "Invalid entity");
		m_Bitset.GetSignature(entity) = signature;
	}
	void EntityManager::Clear()
	{
		m_Bitset.Clear();
		m_Versions.clear();
		m_EntitiesInUse = 0;
	}
	bool EntityManager::IsValid(Entity entity) const
	{
		if (entity >= m_Versions.size() || (uint32_t)entity == 0)
			return false;
		return m_Bitset.IsValid(static_cast<int32_t>(entity));
	}
}