#include "stdafx.h"
#include "CallbackManager.h"

namespace XYZ {
	CallbackManager::CallbackManager()
		:
		m_StoragePool(sc_InitialStorageCapacity * sizeof(CallbackStorage<IComponent>)),
		m_Count(sc_InitialStorageCapacity),
		m_NumberOfStorages(0)
	{
		m_StorageCreated.resize(m_Count);
		for (auto&& it : m_StorageCreated)
			it = false;
	}
	CallbackManager::CallbackManager(const CallbackManager& other)
		:
		m_StoragePool(sc_InitialStorageCapacity * sizeof(CallbackStorage<IComponent>)),
		m_StorageCreated(other.m_StorageCreated),
		m_Count(other.m_Count),
		m_NumberOfStorages(other.m_NumberOfStorages)
	{
		for (size_t i = 0; i < m_Count; ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(CallbackStorage<IComponent>);
				other.GetIStorage(offset)->Copy(&m_StoragePool[offset]);
			}
		}
	}
	CallbackManager::CallbackManager(CallbackManager&& other) noexcept
		:
		m_StoragePool(std::move(other.m_StoragePool)),
		m_StorageCreated(std::move(other.m_StorageCreated)),
		m_Count(other.m_Count),
		m_NumberOfStorages(other.m_NumberOfStorages)
	{
		other.m_Count = 0;
		other.m_NumberOfStorages = other.m_NumberOfStorages;
	}
	CallbackManager::~CallbackManager()
	{
		deallocateStorages();
	}
	CallbackManager& CallbackManager::operator=(CallbackManager&& other) noexcept
	{
		deallocateStorages();
		m_StoragePool = std::move(other.m_StoragePool);
		m_StorageCreated = std::move(other.m_StorageCreated);
		m_NumberOfStorages = other.m_NumberOfStorages;
		return *this;
	}
	void CallbackManager::Clear()
	{
		for (size_t i = 0; i < m_Count; ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(CallbackStorage<IComponent>);
				GetIStorage(offset)->Clear();
			}
		}
	}
	void CallbackManager::OnEntityDestroyed(uint32_t entity, const Signature& signature)
	{
		for (uint32_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (signature[i] && m_StorageCreated[i])
			{
				auto storage = m_StoragePool.Get<ICallbackStorage>(i * sizeof(CallbackStorage<IComponent>));
				storage->Execute(entity, CallbackType::EntityDestroy);
			}
		}
	}
	void CallbackManager::resizeStorages(size_t count)
	{
		size_t capacityBytes = count * sizeof(CallbackStorage<IComponent>);
		Pool newPool(capacityBytes);
		size_t offset = 0;
		for (bool created : m_StorageCreated)
		{
			if (created)
				m_StoragePool.Get<ICallbackStorage>(offset)->Move(&newPool[offset]);
			offset += sizeof(CallbackStorage<IComponent>);
		}
		m_StoragePool = std::move(newPool);

		m_StorageCreated.resize(count);
		std::fill(m_StorageCreated.begin() + m_Count, m_StorageCreated.end(), false);
		m_Count = count;
	}
	void CallbackManager::deallocateStorages()
	{
		for (size_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(CallbackStorage<IComponent>);
				m_StoragePool.Destroy<ICallbackStorage>(m_StoragePool.Get<ICallbackStorage>(offset));
			}
		}
	}
}