#include "stdafx.h"
#include "ComponentManager.h"


namespace XYZ {
	ComponentManager::ComponentManager()
		:
		m_StoragePool(sizeof(ComponentStorage<IComponent>) * MAX_COMPONENTS)
	{
		m_StorageCreated.resize(MAX_COMPONENTS);
		for (auto& it : m_StorageCreated)
			it = false;
	}
	ComponentManager::~ComponentManager()
	{
		
	}
	void ComponentManager::EntityDestroyed(uint32_t entity, const Signature& signature)
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
}