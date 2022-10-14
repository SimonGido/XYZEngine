#include "stdafx.h"
#include "PushConstBuffer.h"

namespace XYZ {
    PushConstBuffer::PushConstBuffer()
    {
        memset(Bytes, 0, sc_MaxSize);
    }
}