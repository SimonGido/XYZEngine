#include "stdafx.h"
#include "EntityManager.h"

namespace XYZ {
	EntityManager::EntityManager()
		:
		m_EntitiesInUse(0)
	{
		// Invalid
		m_Entities.Insert({});
		m_Versions.Insert({});
	}
	EntityManager::EntityManager(const EntityManager& other)
		:
		m_Entities(other.m_Entities),
		m_Versions(other.m_Versions),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager::EntityManager(EntityManager&& other) noexcept
		:
		m_Entities(std::move(other.m_Entities)),
		m_Versions(std::move(other.m_Versions)),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager& EntityManager::operator=(const EntityManager& other)
	{
		m_Entities		= other.m_Entities;
		m_Versions		= other.m_Versions;
		m_EntitiesInUse = other.m_EntitiesInUse;
		return *this;
	}
	EntityManager& EntityManager::operator=(EntityManager&& other) noexcept
	{
		m_Entities = std::move(other.m_Entities);
		m_Versions = std::move(other.m_Versions);
		m_EntitiesInUse = other.m_EntitiesInUse;
		return *this;
	}
	Entity EntityManager::CreateEntity()
	{
		m_EntitiesInUse++;
		XYZ_ASSERT(m_EntitiesInUse < sc_MaxEntity, "Too many entities in existence.");
		
		Entity entity = m_Entities.Insert({});
		m_Entities[entity] = entity;

		if (m_Versions.Range() > entity && m_Versions.Valid(entity))
			m_Versions[entity]++;
		else
		{
			int32_t versionIndex = m_Versions.Insert(0);
			XYZ_ASSERT(versionIndex == entity, "");
		}
		return entity;		
	}
	uint32_t EntityManager::GetVersion(Entity entity) const
	{
		return m_Versions[entity];
	}
	
	void EntityManager::DestroyEntity(Entity entity)
	{
		XYZ_ASSERT(entity, "Invalid entity.");

		m_Entities.Erase(entity);
		m_EntitiesInUse--;
	}


	void EntityManager::Clear()
	{
		m_Entities.Clear();
		m_Versions.Clear();
		m_EntitiesInUse = 0;
	}
	bool EntityManager::IsValid(Entity entity) const
	{
		return m_Entities.Range() > entity && m_Entities.Valid(entity) && entity != Entity(0);
	}
}