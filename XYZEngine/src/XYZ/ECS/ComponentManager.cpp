#include "stdafx.h"
#include "ComponentManager.h"


namespace XYZ {

	uint16_t ComponentManager::s_NextComponentTypeID = 0;

	ComponentManager::ComponentManager()
		:
		m_StoragesCreated(0)
	{
	}
	ComponentManager::ComponentManager(const ComponentManager& other)
		:
		m_StoragesCreated(other.m_StoragesCreated)
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Copy();
			}
			counter++;
		}
	}
	ComponentManager::ComponentManager(ComponentManager&& other) noexcept
		:
		m_StoragesCreated(other.m_StoragesCreated)
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
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
	ComponentManager& ComponentManager::operator=(ComponentManager&& other) noexcept
	{
		destroyStorages();
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
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
	void ComponentManager::EntityDestroyed(Entity entity, const Signature& signature)
	{
		uint32_t counter = 0;
		for (auto storage : m_Storages)
		{
			if (storage && signature[counter])
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
		for (auto storage : m_Storages)
		{
			if (storage)
				delete storage;
		}
		m_Storages.clear();
	}
}