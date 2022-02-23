#include "stdafx.h"
#include "EntityManager.h"

namespace XYZ {
	EntityManager::EntityManager()
		:
		m_EntitiesInUse(0)
	{
		m_Entities.push_back({}); // Invalid entity
	}
	EntityManager::EntityManager(const EntityManager& other)
		:
		m_Entities(other.m_Entities),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager::EntityManager(EntityManager&& other) noexcept
		:
		m_Entities(std::move(other.m_Entities)),
		m_EntitiesInUse(other.m_EntitiesInUse)
	{
	}
	EntityManager& EntityManager::operator=(const EntityManager& other)
	{
		m_Entities = other.m_Entities;
		m_EntitiesInUse = other.m_EntitiesInUse;
		return *this;
	}
	EntityManager& EntityManager::operator=(EntityManager&& other) noexcept
	{
		m_Entities = std::move(other.m_Entities);
		m_EntitiesInUse = other.m_EntitiesInUse;
		return *this;
	}
	Entity EntityManager::CreateEntity()
	{
		m_EntitiesInUse++;
		XYZ_ASSERT(m_EntitiesInUse < sc_MaxEntity, "Too many entities in existence.");

		if (m_Free.empty())
		{
			Entity entity(static_cast<uint32_t>(m_Entities.size()));
			m_Entities.push_back(entity);
			return entity;
		}
		else
		{
			const size_t index = m_Free.front();
			m_Free.pop();
			Entity entity(static_cast<uint32_t>(index));
			return entity;
		}
	}

	void EntityManager::DestroyEntity(Entity entity)
	{
		const size_t index = static_cast<size_t>(entity.m_ID);

		XYZ_ASSERT(IsValid(entity), "Invalid entity.");

		// We increase version right after destroying, so old references will not be valid
		m_Entities[index].m_Version++;
		m_Free.push(index);
		m_EntitiesInUse--;
	}

	void EntityManager::Clear()
	{
		m_Entities.clear();
		m_EntitiesInUse = 0;
	}

	bool EntityManager::IsValid(Entity entity) const
	{
		const size_t index = static_cast<size_t>(entity.m_ID);
		return m_Entities.size() > index
			&& m_Entities[index] == entity
			&& entity;
	}
}