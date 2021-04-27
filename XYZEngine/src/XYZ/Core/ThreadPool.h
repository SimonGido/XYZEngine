#pragma once
#include "XYZ/Utils/DataStructures/Queue.h"

#include <thread>
#include <future>
#include <mutex>
#include <queue>

namespace XYZ {

	class ThreadPool
	{
		using Job = std::function<void()>;
	public:
		ThreadPool(uint32_t numThreads);
		ThreadPool(const ThreadPool& other) = delete;
		~ThreadPool();

		template <typename R, typename F>
		std::future<R> PushJob(F&& f) const
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

		template <typename R, typename F>
		std::future<R> PushJob(F&& f)
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

	private:
		void stop();
		void waitForJob();

	private:
		std::vector<std::thread> m_Threads;

		mutable std::mutex m_Mutex;
		mutable std::queue<Job> m_JobQueue;
		mutable std::condition_variable m_Condition;

		bool m_Terminate;
	};

}