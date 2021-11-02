#pragma once
#include "XYZ/Utils/DataStructures/Queue.h"
#include "XYZ/Utils/DataStructures/FreeList.h"

#include <thread>
#include <future>
#include <mutex>
#include <queue>

namespace XYZ {

	class ThreadPool
	{
		using Job = std::function<void()>;
	public:
		ThreadPool();
		ThreadPool(uint32_t numThreads);
		ThreadPool(const ThreadPool& other) = delete;
		~ThreadPool();


		void PushJob(Job&& f);
		void PushJob(Job&& f) const;

		template <typename R, typename F>
		std::future<R> PushJob(F&& f) const;
		
		template <typename R, typename F>
		std::future<R> PushJob(F&& f);
	
		
		int32_t PushThread();
		void    EraseThread(int32_t index);

		void    Clear();
	private:
		void stop();
		void waitForJob(int32_t id);

	private:
		struct Thread
		{
			Thread(std::thread thread);

			std::thread	  WorkerThread;
			bool		  Terminate;
		};
		FreeList<Thread> m_Threads;

		mutable std::mutex m_Mutex;
		mutable std::queue<Job> m_JobQueue;
		mutable std::condition_variable m_Condition;
	};

	template<typename R, typename F>
	inline std::future<R> ThreadPool::PushJob(F&& f) const
	{
		auto task = std::make_shared< std::packaged_task<R()> >(
			std::bind(std::forward<F>(f))
			);
		std::future<R> future = task->get_future();
		{
			std::scoped_lock<std::mutex> lock(m_Mutex);
			m_JobQueue.emplace([task] { (*task)(); });
		}
		m_Condition.notify_one();
		return future;
	}

	template<typename R, typename F>
	inline std::future<R> ThreadPool::PushJob(F&& f)
	{
		auto task = std::make_shared< std::packaged_task<R()> >(
			std::bind(std::forward<F>(f))
			);
		std::future<R> future = task->get_future();
		{
			std::scoped_lock<std::mutex> lock(m_Mutex);
			m_JobQueue.emplace([task] { (*task)(); });
		}
		m_Condition.notify_one();
		return future;
	}

}