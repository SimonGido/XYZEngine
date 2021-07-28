#pragma once

#include <mutex>
#include <deque>

namespace XYZ {
	namespace Net {

		template <typename T>
		class Queue
		{
		public:
			Queue() = default;
			Queue(const Queue<T>&) = delete;
			virtual ~Queue() { Clear(); }

			const T& Front()
			{
				std::scoped_lock lock(m_MutQueue);
				return m_Queue.front();
			}

			const T& Back()
			{
				std::scoped_lock lock(m_MutQueue);
				return m_Queue.back();
			}

			void PushBack(const T& elem)
			{
				std::scoped_lock lock(m_MutQueue);
				m_Queue.emplace_back(std::move(elem));

				std::unique_lock<std::mutex>(m_MutBlocking);
				m_Blocking.notify_one();
			}

			void PushFront(const T& elem)
			{
				std::scoped_lock lock(m_MutQueue);
				m_Queue.emplace_front(std::move(elem));

				std::unique_lock<std::mutex> (m_MutBlocking);
				m_Blocking.notify_one();
			}

			bool Empty()
			{
				std::scoped_lock lock(m_MutQueue);
				return m_Queue.empty();
			}

			size_t Size()
			{
				std::scoped_lock lock(m_MutQueue);
				return m_Queue.size();
			}

			void Clear()
			{
				std::scoped_lock lock(m_MutQueue);
				m_Queue.clear();
			}

			T PopFront()
			{
				std::scoped_lock lock(m_MutQueue);
				T temp = std::move(m_Queue.front());
				m_Queue.pop_front();
				return temp;
			}

			T PopBack()
			{
				std::scoped_lock lock(m_MutQueue);
				T temp = std::move(m_Queue.back());
				m_Queue.pop_back();
				return temp;
			}

			void Wait()
			{
				while (Empty())
				{
					std::unique_lock<std::mutex> ul(m_MutBlocking);
					m_Blocking.wait(ul);
				}
			}

		private:
			std::deque<T>			m_Queue;
			std::condition_variable m_Blocking;
			std::mutex				m_MutBlocking;
			std::mutex				m_MutQueue;
		};
	}
}