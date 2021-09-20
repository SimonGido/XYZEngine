#include "stdafx.h"
#include "ComponentWrapper.h"

namespace XYZ {
    AnyComponentReference::AnyComponentReference()
        :
        m_Reference(nullptr),
        m_ID(std::numeric_limits<uint16_t>::max())
    {
    }
}