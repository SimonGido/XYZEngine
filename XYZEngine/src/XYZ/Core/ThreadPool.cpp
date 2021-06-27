#include "stdafx.h"
#include "ThreadPool.h"



namespace XYZ {
	ThreadPool::ThreadPool(uint32_t numThreads)
		:
		m_Terminate(false)
	{
		if (numThreads > std::thread::hardware_concurrency())
			XYZ_LOG_WARN("Creating more threads than the maximum number of threads");
		for (uint32_t i = 0; i < numThreads; ++i)
			m_Threads.push_back(std::thread(&ThreadPool::waitForJob, this));
	}
	ThreadPool::~ThreadPool()
	{
		stop();
	}
	
	void ThreadPool::stop()
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Terminate = true;
		}
		m_Condition.notify_all(); // wake up all threads.

		for (std::thread& thread : m_Threads)
		{
			thread.join();
		}
		m_Threads.clear();
	}
	void ThreadPool::waitForJob()
	{
		Job job;
		while (true)
		{		
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Condition.wait(lock, [&] {return !m_JobQueue.empty() || m_Terminate; });
				if (m_Terminate && m_JobQueue.empty())
					return;

				job = std::move(m_JobQueue.front());
				m_JobQueue.pop();
			}
			job();		
		}
	}
}