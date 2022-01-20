#include "stdafx.h"
#include "ECSManager.h"


namespace XYZ {
	ECSManager::ECSManager(const ECSManager& other)
		:
		m_ComponentManager(other.m_ComponentManager),
		m_EntityManager(other.m_EntityManager),
		m_OnConstruction(other.m_OnConstruction),
		m_OnDestruction(other.m_OnDestruction)
	{
	}
	ECSManager::ECSManager(ECSManager&& other) noexcept
		:
		m_ComponentManager(std::move(other.m_ComponentManager)),
		m_EntityManager(std::move(other.m_EntityManager)),
		m_OnConstruction(std::move(other.m_OnConstruction)),
		m_OnDestruction(std::move(other.m_OnDestruction))
	{
	}
	ECSManager& ECSManager::operator=(const ECSManager& other)
	{
		m_ComponentManager  = other.m_ComponentManager;
		m_EntityManager		= other.m_EntityManager;
		m_OnConstruction	= other.m_OnConstruction;
		m_OnDestruction		= other.m_OnDestruction;
		return *this;
	}
	ECSManager& ECSManager::operator=(ECSManager&& other) noexcept
	{
		m_ComponentManager = std::move(other.m_ComponentManager);
		m_EntityManager = std::move(other.m_EntityManager);
		m_OnConstruction = std::move(other.m_OnConstruction);
		m_OnDestruction = std::move(other.m_OnDestruction);
		return *this;
	}
	Entity ECSManager::CopyEntity(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Accesing invalid entity");
		Entity result = m_EntityManager.CreateEntity();

		for (const auto storage : m_ComponentManager.GetIStorages())
		{
			if (storage)
			{
				if (HasComponent(entity, storage->ID()))
				{
					storage->CopyEntity(entity, result);
				}
			}
		}
		executeOnConstruction(entity);
		return result;
	}
	Entity ECSManager::CreateEntity()
	{ 
		return m_EntityManager.CreateEntity(); 
	}
	void ECSManager::DestroyEntity(Entity entity)
	{ 
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		m_ComponentManager.EntityDestroyed(entity);
		m_EntityManager.DestroyEntity(entity); 
		executeOnDestruction(entity);
	}
	void ECSManager::Clear()
	{
		m_ComponentManager.Clear();
		m_EntityManager.Clear();
		m_OnConstruction.clear();
		m_OnDestruction.clear();
	}

	bool ECSManager::HasComponent(Entity entity, uint16_t componentID) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		return GetIStorage(componentID)->HasEntity(entity);
	}
	bool ECSManager::IsValid(Entity entity) const
	{
		return m_EntityManager.IsValid(entity);
	}
	IComponentStorage* ECSManager::GetIStorage(uint16_t index)
	{
		return m_ComponentManager.GetIStorage(index);
	}
	const IComponentStorage* ECSManager::GetIStorage(uint16_t index) const
	{
		return m_ComponentManager.GetIStorage(index);
	}
	void ECSManager::executeOnConstruction(Entity entity)
	{
		for (auto& callback : m_OnConstruction)
			callback(entity);
	}
	void ECSManager::executeOnDestruction(Entity entity)
	{
		for (auto& callback : m_OnDestruction)
			callback(entity);
	}
}