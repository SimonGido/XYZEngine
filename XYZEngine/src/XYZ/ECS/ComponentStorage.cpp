#include "stdafx.h"
#include "ComponentStorage.h"


namespace XYZ {
    IComponentStorage::IComponentStorage(const IComponentStorage& other)
        :
        m_OnConstruction(other.m_OnConstruction),
        m_OnDestruction(other.m_OnDestruction)
    {
    }
    IComponentStorage::IComponentStorage(IComponentStorage&& other) noexcept
        :
        m_OnConstruction(std::move(other.m_OnConstruction)),
        m_OnDestruction(std::move(other.m_OnDestruction))
    {
    }
}