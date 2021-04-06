#include "stdafx.h"
#include "CallbackManager.h"

namespace XYZ {
	CallbackManager::CallbackManager()
		:
		m_StoragePool(sizeof(CallbackStorage<IComponent>) * MAX_COMPONENTS)
	{
		m_StorageCreated.resize(MAX_COMPONENTS);
		for (auto&& it : m_StorageCreated)
			it = false;
	}
	CallbackManager::CallbackManager(const CallbackManager& other)
		:
		m_StoragePool(sizeof(CallbackStorage<IComponent>) * MAX_COMPONENTS),
		m_StorageCreated(other.m_StorageCreated)
	{
		for (size_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (m_StorageCreated[i])
			{
				size_t offset = i * sizeof(ICallbackStorage);		
				other.GetIStorage(offset)->Copy(&m_StoragePool.GetRawData()[offset]);
			}
		}
	}
	CallbackManager::~CallbackManager()
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
	void CallbackManager::OnEntityDestroyed(uint32_t entity, const Signature& signature)
	{
		for (uint32_t i = 0; i < m_StorageCreated.size(); ++i)
		{
			if (signature.test(i) && m_StorageCreated[i])
			{
				auto storage = m_StoragePool.Get<ICallbackStorage>(i * sizeof(CallbackStorage<IComponent>));
				storage->Execute(entity, CallbackType::EntityDestroy);
			}
		}
	}
}