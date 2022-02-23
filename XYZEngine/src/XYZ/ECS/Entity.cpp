#include "stdafx.h"
#include "Entity.h"


namespace XYZ {

    Entity::Entity()
        :
        m_ID(0),
        m_Version(0)
    {
    }
    Entity::Entity(uint32_t id)
        : 
        m_ID(id), 
        m_Version(0)
    {
    }
    Entity::Entity(const Entity& other)
        : 
        m_ID(other.m_ID),
        m_Version(other.m_Version)
    {
    }
    Entity& Entity::operator=(const Entity& other)
    {
        m_ID = other.m_ID;
        m_Version = other.m_Version;
        return *this;
    }
    bool Entity::operator==(const Entity& rhs) const
    {
        return m_ID == rhs.m_ID && m_Version == rhs.m_Version;
    }
    bool Entity::operator!=(const Entity& rhs) const
    {
        return m_ID != rhs.m_ID || m_Version != rhs.m_Version;
    }
  
    Entity::operator uint32_t() const
    {
        return m_ID;
    }
    uint64_t Entity::GetHash() const
    {
        uint64_t id = static_cast<uint64_t>(m_ID) | (static_cast<uint64_t>(m_Version) << 32);
        const std::hash<uint64_t> hasher;
        return hasher(id);
    }
}