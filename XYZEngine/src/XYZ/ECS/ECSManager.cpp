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
}