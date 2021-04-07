#include "stdafx.h"
#include "ComponentManager.h"


namespace XYZ {
	ComponentManager::ComponentManager()
		:
		m_StoragePool(sizeof(ComponentStorage<IComponent>) * MAX_COMPONENTS)
	{
		m_StorageCreated.resize(MAX_COMPONENTS);
		for (auto&& it : m_StorageCreated)
			it = false;
	}
	ComponentManager::ComponentManager(const ComponentManager& other)
		:
		m_StoragePool(sizeof(ComponentStorage<IComponent>) * MAX_COMPONENTS)
	{
		m_StorageCreated = other.m_StorageCreated;
		for (size_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(ComponentStorage<IComponent>);
				other.GetIStorage(offset)->Copy(&m_StoragePool.GetRawData()[offset]);
			}
		}
	}
	ComponentManager::ComponentManager(ComponentManager&& other) noexcept
		:
		m_StoragePool(std::move(other.m_StoragePool)),
		m_StorageCreated(std::move(other.m_StorageCreated)),
		m_NumberOfStorages(other.m_NumberOfStorages)
	{
	}
	ComponentManager::~ComponentManager()
	{
		deallocateStorages();
	}
	ComponentManager& ComponentManager::operator=(ComponentManager&& other) noexcept
	{
		deallocateStorages();
		m_StoragePool = std::move(other.m_StoragePool);
		m_StorageCreated = std::move(other.m_StorageCreated);
		m_NumberOfStorages = other.m_NumberOfStorages;
		return *this;
	}
	void ComponentManager::EntityDestroyed(Entity entity, const Signature& signature)
	{
		for (uint32_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (signature.test(i) && m_StorageCreated[i])
			{
				auto storage = m_StoragePool.Get<IComponentStorage>(i * sizeof(ComponentStorage<IComponent>));
				storage->EntityDestroyed(entity);
			}
		}
	}
	void ComponentManager::deallocateStorages()
	{
		for (size_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(ComponentStorage<IComponent>);
				m_StoragePool.Destroy<IComponentStorage>(m_StoragePool.Get<IComponentStorage>(offset));
				m_StorageCreated[i] = false;
			}
		}
	}
}