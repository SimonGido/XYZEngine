#include "stdafx.h"
#include "ECSManager.h"


namespace XYZ {
	ECSManager::ECSManager(const ECSManager& other)
		:
		m_ComponentManager(other.m_ComponentManager),
		m_EntityManager(other.m_EntityManager)
	{
	}
	ECSManager::ECSManager(ECSManager&& other) noexcept
		:
		m_ComponentManager(std::move(other.m_ComponentManager)),
		m_EntityManager(std::move(other.m_EntityManager))
	{
	}
	ECSManager& ECSManager::operator=(ECSManager&& other) noexcept
	{
		m_ComponentManager = std::move(other.m_ComponentManager);
		m_EntityManager = std::move(other.m_EntityManager);
		return *this;
	}
	Entity ECSManager::CopyEntity(Entity entity)
	{
		XYZ_ASSERT(IsValid(entity), "Accesing invalid entity");
		Entity result = m_EntityManager.CreateEntity();
		for (auto storage : m_ComponentManager.m_Storages)
		{
			ByteStream out;
			storage->CopyComponentData(entity, out);
			storage->AddRawComponent(result, out);
		}
		return result;
	}
	Entity ECSManager::CreateEntity()
	{ 
		return m_EntityManager.CreateEntity(); 
	}
	void ECSManager::DestroyEntity(Entity entity)
	{ 
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		auto& signature = m_EntityManager.GetSignature(entity);
		m_ComponentManager.EntityDestroyed(entity, signature);
		m_EntityManager.DestroyEntity(entity); 
	}
	void ECSManager::Clear()
	{
		m_ComponentManager.Clear();
		m_EntityManager.Clear();
	}
	const Signature& ECSManager::GetEntitySignature(Entity entity) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		return m_EntityManager.GetSignature(entity);
	}
	bool ECSManager::Contains(Entity entity, uint16_t componentID) const
	{
		XYZ_ASSERT(IsValid(entity), "Entity is invalid");
		auto& signature = m_EntityManager.GetSignature(entity);
		if (signature.Size() <= componentID)
			return false;
		return signature[componentID];
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
}