#include "stdafx.h"
#include "ComponentStorage.h"

#include "ECSManager.h"

namespace XYZ {
    IComponentStorage::IComponentStorage(ECSManager& ecs)
        :
        m_ECS(&ecs)
    {
    }
    IComponentStorage::IComponentStorage(const IComponentStorage& other)
        :
        m_OnConstruction(other.m_OnConstruction),
        m_OnDestruction(other.m_OnDestruction),
        m_ECS(other.m_ECS)
    {
    }
    IComponentStorage::IComponentStorage(IComponentStorage&& other) noexcept
        :
        m_OnConstruction(std::move(other.m_OnConstruction)),
        m_OnDestruction(std::move(other.m_OnDestruction)),
        m_ECS(other.m_ECS)
    {
    }
}