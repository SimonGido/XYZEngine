#include "stdafx.h"
#include "ThreadPool.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {
	ThreadPool::ThreadPool()
		:
		m_Running(false),
		m_Waiting(false)
	{
	}

	ThreadPool::~ThreadPool()
	{
		Stop();
	}


	void ThreadPool::Start(uint32_t numThreads)
	{
		if (!m_Running)
		{
			m_Running = true;
			if (numThreads > std::thread::hardware_concurrency())
				XYZ_CORE_WARN("Creating more threads than the maximum number of threads");
			{
				std::unique_lock<std::mutex> lock(m_JobMutex);
				for (uint32_t i = 0; i < numThreads; ++i)
					m_Threads.push_back(std::thread(&ThreadPool::worker, this));
			}
		}
	}

	void ThreadPool::Stop()
	{
		if (m_Running)
		{
			WaitForJobs();
			m_Running = false;
			m_JobAvailableCV.notify_all(); // wake up all threads.

			for (size_t i = 0; i < m_Threads.size(); ++i)
			{
				m_Threads[i].join();
			}
			m_Threads.clear();
		}
	}
	void ThreadPool::WaitForJobs()
	{
		m_Waiting = true;
		std::unique_lock<std::mutex> lock(m_JobMutex);
		m_JobDoneCV.wait(lock, [this] { return m_JobQueue.empty(); });
		m_Waiting = false;
	}
	void ThreadPool::worker()
	{
		Job job;
		XYZ_PROFILE_THREAD("WorkerThread");

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_JobMutex);
				m_JobAvailableCV.wait(lock, [&] { return !m_JobQueue.empty() || !m_Running; });
				if (!m_Running && m_JobQueue.empty())
					return;

				job = std::move(m_JobQueue.front());
				m_JobQueue.pop();
			}
			job();		
		}
	}
	
}