#include "stdafx.h"
#include "ThreadPool.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {
	ThreadPool::ThreadPool()
	{
	}
	ThreadPool::ThreadPool(uint32_t numThreads)
	{
		if (numThreads > std::thread::hardware_concurrency())
			XYZ_CORE_WARN("Creating more threads than the maximum number of threads");
		for (uint32_t i = 0; i < numThreads; ++i)
			m_Threads.Emplace(std::thread(&ThreadPool::waitForJob, this, i));
	}
	ThreadPool::~ThreadPool()
	{
		stop();
	}
	
	int32_t ThreadPool::PushThread()
	{
		int32_t id = m_Threads.Next();
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Threads.Emplace(std::move(std::thread(&ThreadPool::waitForJob, this, id)));
		}
		return id;
	}

	void ThreadPool::EraseThread(int32_t index)
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Threads[index].Terminate = true;
		}
		m_Condition.notify_all(); // wake up all threads.

		m_Threads[index].WorkerThread.join();
		m_Threads.Erase(index);
	}

	void ThreadPool::Clear()
	{
		stop();
	}

	void ThreadPool::stop()
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			for (int32_t i = 0; i < m_Threads.Range(); ++i)
			{
				if (m_Threads.Valid(i))
					m_Threads[i].Terminate = true;
			}
		}
		m_Condition.notify_all(); // wake up all threads.

		for (int32_t i = 0; i < m_Threads.Range(); ++i)
		{
			if (m_Threads.Valid(i))
				m_Threads[i].WorkerThread.join();
		}
		m_Threads.Clear();
	}
	void ThreadPool::waitForJob(int32_t id)
	{
		Job job;
		size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		XYZ_PROFILE_THREAD("WorkerThread");

		while (true)
		{
			XYZ_PROFILE_FRAME("WorkerThread");
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_Condition.wait(lock, [&] {
					return !m_JobQueue.empty() || m_Threads[id].Terminate;
				});
				if (m_Threads[id].Terminate && m_JobQueue.empty())
					return;

				job = std::move(m_JobQueue.front());
				m_JobQueue.pop();
			}
			job();		
		}
	}
	ThreadPool::Thread::Thread(std::thread thread)
		:
		WorkerThread(std::move(thread)),
		Terminate(false)
	{}
	
}