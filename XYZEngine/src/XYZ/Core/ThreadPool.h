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
		ThreadPool(const ThreadPool& other) = delete;
		~ThreadPool();

		void Start(uint32_t numThreads);
		void Stop();

	
		template <typename F, typename... A>
		void PushJob(F&& task, A&&... args);
		
		template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
		std::future<R> SubmitJob(F&& task, A&&... args);	

	private:	
		void waitForJob();

	private:
		std::atomic_bool		 m_Running;
		std::vector<std::thread> m_Threads;

		mutable std::mutex m_Mutex;
		mutable std::queue<Job> m_JobQueue;
		mutable std::condition_variable m_Condition;
	};


	template<typename F, typename ...A>
	inline void ThreadPool::PushJob(F&& task, A && ...args)
	{
		Job taskFunction = std::bind(std::forward<F>(task), std::forward<A>(args)...);
		{
			const std::scoped_lock tasksLock(m_Mutex);
			m_JobQueue.push(taskFunction);
		}
		m_Condition.notify_one();
	}

	template<typename F, typename ...A, typename R>
	inline std::future<R> ThreadPool::SubmitJob(F&& task, A && ...args)
	{
		std::function<R()> taskFunction = std::bind(std::forward<F>(task), std::forward<A>(args)...);
		std::shared_ptr<std::promise<R>> taskPromise = std::make_shared<std::promise<R>>();
		
		PushJob([taskFunction, taskPromise] {			
			if constexpr (std::is_void_v<R>)
			{
				std::invoke(task_function);
				taskPromise->set_value();
			}
			else
			{
				taskPromise->set_value(std::invoke(taskFunction));
			}			
		});
		return taskPromise->get_future();
	}

}