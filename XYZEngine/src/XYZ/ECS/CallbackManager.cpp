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