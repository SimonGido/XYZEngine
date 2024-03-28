#pragma once

#include <shared_mutex>
#include <thread>

namespace XYZ{
	
	class XYZ_API SharedRecursiveMutex : public std::shared_mutex
	{
	public:
        void lock();
        void unlock();

    private:
        std::atomic<std::thread::id> m_Owner;
        uint32_t m_Count;
    };

}
