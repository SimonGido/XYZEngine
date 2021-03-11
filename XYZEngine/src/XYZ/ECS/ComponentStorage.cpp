#include "stdafx.h"
#include "ComponentStorage.h"


namespace XYZ {
    ComponentStorage::ComponentStorage()
        : m_Data(INVALID_COMPONENT)
    {
    }
    ComponentStorage::ComponentStorage(uint8_t id)
        : m_Data(id)
    {}
}