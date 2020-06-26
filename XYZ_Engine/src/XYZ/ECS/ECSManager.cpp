#include "stdafx.h"
#include "ECSManager.h"

namespace XYZ {
	ECSManager::ECSManager(token)
	{
		m_ComponentManager = std::make_unique<ComponentManager>();
		m_EntityManager = std::make_unique<EntityManager>();
		m_SystemManager = std::make_unique<SystemManager>();
	}
}