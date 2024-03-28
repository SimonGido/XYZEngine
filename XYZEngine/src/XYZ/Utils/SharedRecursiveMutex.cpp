#include "stdafx.h"
#include "SharedRecursiveMutex.h"


namespace XYZ {
	void SharedRecursiveMutex::lock()
    {
        std::thread::id this_id = std::this_thread::get_id();
        if (m_Owner == this_id) 
        {
            // recursive locking
            m_Count++;
        }
        else 
        {
            // normal locking
            shared_mutex::lock();
            m_Owner = this_id;
            m_Count = 1;
        }
    }
    void SharedRecursiveMutex::unlock()
    {
        if (m_Count > 1) 
        {
            // recursive unlocking
            m_Count--;
        }
        else 
        {
            // normal unlocking
            m_Owner = std::thread::id();
            m_Count = 0;
            shared_mutex::unlock();
        }
    }
}
