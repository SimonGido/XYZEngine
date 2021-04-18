#include "stdafx.h"
#include "Entity.h"


namespace XYZ {

    Entity::Entity()
        :
        m_ID(0)
    {
    }
    Entity::Entity(uint32_t id)
        : m_ID(id)
    {
    }
    Entity::Entity(const Entity& other)
        : m_ID(other.m_ID)
    {
    }
    Entity& Entity::operator=(const Entity& other)
    {
        m_ID = other.m_ID;
        return *this;
    }
    bool Entity::operator==(const Entity& rhs) const
    {
        return m_ID == rhs.m_ID;
    }
    bool Entity::operator!=(const Entity& rhs) const
    {
        return m_ID != rhs.m_ID;
    }
  
    Entity::operator uint32_t() const
    {
        return m_ID;
    }
}