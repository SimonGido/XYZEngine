#include "stdafx.h"
#include "ComponentManager.h"


namespace XYZ {

	uint16_t ComponentManager::s_NextComponentTypeID = 0;

	ComponentManager::ComponentManager()
	{
	}
	ComponentManager::ComponentManager(const ComponentManager& other)
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (const auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Copy();
			}
			counter++;
		}
	}
	ComponentManager::ComponentManager(ComponentManager&& other) noexcept
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (const auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Move();
			}
			counter++;
		}
		other.m_Storages.clear();
	}
	ComponentManager::~ComponentManager()
	{
		destroyStorages();
	}
	ComponentManager& ComponentManager::operator=(const ComponentManager& other) noexcept
	{
		destroyStorages();
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (const auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Copy();
			}
			counter++;
		}
		return *this;
	}
	ComponentManager& ComponentManager::operator=(ComponentManager&& other) noexcept
	{
		destroyStorages();
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (const auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Move();
			}
			counter++;
		}
		other.m_Storages.clear();
		return *this;
	}
	void ComponentManager::EntityDestroyed(Entity entity)
	{
		uint32_t counter = 0;
		for (const auto storage : m_Storages)
		{
			if (storage && storage->HasEntity(entity))
			{
				storage->EntityDestroyed(entity);
			}
			counter++;
		}
	}

	void ComponentManager::Clear()
	{
		destroyStorages();
	}

	void ComponentManager::destroyStorages()
	{
		for (const auto storage : m_Storages)
		{
			if (storage)
				delete storage;
		}
		m_Storages.clear();
	}
}