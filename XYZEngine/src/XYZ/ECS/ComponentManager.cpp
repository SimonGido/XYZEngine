#include "stdafx.h"
#include "ComponentManager.h"


namespace XYZ {
	ComponentManager::ComponentManager()
		:
		m_StoragePool(sc_InitialStorageCapacity * sizeof(ComponentStorage<IComponent>)),
		m_Count(sc_InitialStorageCapacity),
		m_NumberOfStorages(0)
	{
		m_StorageCreated.resize(m_Count);
		for (auto&& it : m_StorageCreated)
			it = false;
	}
	ComponentManager::ComponentManager(const ComponentManager& other)
		:
		m_StoragePool(other.m_Count),
		m_Count(other.m_Count),
		m_NumberOfStorages(other.m_NumberOfStorages)
	{
		m_StorageCreated = other.m_StorageCreated;
		for (size_t i = 0; i < m_Count; ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(ComponentStorage<IComponent>);
				other.GetIStorage(offset)->Copy(&m_StoragePool[offset]);
			}
		}
	}
	ComponentManager::ComponentManager(ComponentManager&& other) noexcept
		:
		m_StoragePool(std::move(other.m_StoragePool)),
		m_StorageCreated(std::move(other.m_StorageCreated)),
		m_Count(other.m_Count),
		m_NumberOfStorages(other.m_NumberOfStorages)
	{
		other.m_Count = 0;
		other.m_NumberOfStorages = 0;
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
		// TODO: Bug
		uint32_t counter = 0;
		for (uint32_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (m_StorageCreated[i])
			{
				if (signature[counter])
				{
					auto storage = m_StoragePool.Get<IComponentStorage>(i * sizeof(ComponentStorage<IComponent>));
					storage->EntityDestroyed(entity);
				}
				counter++;
			}
		}
	}
	void ComponentManager::Clear()
	{
		for (size_t i = 0; i < m_Count; ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(ComponentStorage<IComponent>);
				GetIStorage(offset)->Clear();
			}
		}
	}
	void ComponentManager::resizeStorages(size_t count)
	{
		size_t capacityBytes = count * sizeof(ComponentStorage<IComponent>);
		Pool newPool(capacityBytes);
		size_t offset = 0;
		for (bool created : m_StorageCreated)
		{
			if (created)
				m_StoragePool.Get<IComponentStorage>(offset)->Move(&newPool[offset]);
			offset += sizeof(ComponentStorage<IComponent>);
		}
		m_StoragePool = std::move(newPool);
		m_StorageCreated.resize(count);
		std::fill(m_StorageCreated.begin() + m_Count, m_StorageCreated.end(), false);
		m_Count = count;
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