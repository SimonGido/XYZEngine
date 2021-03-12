#include "stdafx.h"
#include "ComponentStorage.h"


namespace XYZ {

    void ComponentStorage::Init(uint8_t id, size_t elementSize)
    {
        m_Data.Init(id, elementSize);
    }
}