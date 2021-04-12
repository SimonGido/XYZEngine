#include "stdafx.h"
#include "ECSManager.h"


namespace XYZ {
	ECSManager::ECSManager(const ECSManager& other)
		:
		m_ComponentManager(other.m_ComponentManager),
		m_CallbackManager(other.m_CallbackManager),
		m_EntityManager(other.m_EntityManager)
	{
	}
	ECSManager::ECSManager(ECSManager&& other) noexcept
		:
		m_ComponentManager(std::move(other.m_ComponentManager)),
		m_CallbackManager(std::move(other.m_CallbackManager)),
		m_EntityManager(std::move(other.m_EntityManager))
	{
	}
	ECSManager& ECSManager::operator=(ECSManager&& other) noexcept
	{
		m_ComponentManager = std::move(other.m_ComponentManager);
		m_CallbackManager = std::move(other.m_CallbackManager);
		m_EntityManager = std::move(other.m_EntityManager);
		return *this;
	}
	uint32_t ECSManager::CreateEntity()
	{ 
		return m_EntityManager.CreateEntity(); 
	}
	void ECSManager::DestroyEntity(Entity entity)
	{ 
		auto& signature = m_EntityManager.GetSignature(entity);
		m_CallbackManager.OnEntityDestroyed(entity, signature);
		m_ComponentManager.EntityDestroyed(entity, signature);
		m_EntityManager.DestroyEntity(entity); 
	}
	void ECSManager::Clear()
	{
		m_ComponentManager.Clear();
		m_EntityManager.Clear();
		m_CallbackManager.Clear();
	}
}