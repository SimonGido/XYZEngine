#include "stdafx.h"
#include "BasicUIQueue.h"

namespace XYZ {
    bUIQueue::bUIQueue(uint32_t size)
        :
        m_Next(0)
    {
        m_Buffer.Allocate(size);
    }
    bUIQueue::~bUIQueue()
    {
        delete[]m_Buffer;
    }
}