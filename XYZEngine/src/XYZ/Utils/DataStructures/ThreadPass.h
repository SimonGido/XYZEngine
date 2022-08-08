#pragma once
#include "ScopedLock.h"

#include <mutex>
#include <shared_mutex>

namespace XYZ {

	template <typename T>
	class DoubleBuffer
	{
	public:
		DoubleBuffer(size_t size);

		void Write(size_t index, const T& val);

		void Read(size_t index, T& val) const;

		void Swap();

	private:
		std::vector<T>  m_Buffer[2];
		std::vector<T>* m_Write;
		std::vector<T>* m_Read;

		mutable std::mutex m_ReadMutex;
		std::mutex		   m_WriteMutex;
	};

	template<typename T>
	inline DoubleBuffer<T>::DoubleBuffer(size_t size)
	{
		m_Buffer[0].resize(size);
		m_Buffer[1].resize(size);

		m_Write = &m_Buffer[0];
		m_Read = &m_Buffer[1];
	}

	template<typename T>
	inline void DoubleBuffer<T>::Write(size_t index, const T& val)
	{
		std::scoped_lock<std::mutex> lock(m_WriteMutex);
		(*m_Write)[index] = val;
	}

	template<typename T>
	inline void DoubleBuffer<T>::Read(size_t index, T& val) const
	{
		std::scoped_lock<std::mutex> lock(m_ReadMutex);
		val = (*m_Read)[index];
	}

	template<typename T>
	inline void DoubleBuffer<T>::Swap()
	{
		std::scoped_lock<std::mutex> lock(m_WriteMutex, m_ReadMutex);
		std::swap(m_Read, m_Write);
	}



	template <typename T>
	class ThreadPass
	{
	public:
		ThreadPass();
		ThreadPass(const ThreadPass<T>& other);
		ThreadPass(ThreadPass<T>&& other) noexcept;

		ThreadPass& operator=(const ThreadPass<T>& other);
		ThreadPass& operator=(ThreadPass<T>&& other) noexcept;

		ScopedLock<T> Write();
		ScopedLock<T> Read();

		ScopedLockRead<T> WriteRead() const;
		ScopedLockRead<T> ReadRead() const;

		void Swap();
		bool AttemptSwap();

	private:
		T  m_Data0;
		T  m_Data1;
		T* m_Read;
		T* m_Write;

		mutable std::shared_mutex m_ReadMutex;
		mutable std::shared_mutex m_WriteMutex;
	};


	template<typename T>
	inline ThreadPass<T>::ThreadPass()
	{
		m_Write = &m_Data0;
		m_Read  = &m_Data1;
	}

	template<typename T>
	inline ThreadPass<T>::ThreadPass(const ThreadPass<T>& other)
	{
		std::shared_lock otherReadLock(other.m_ReadMutex);
		std::shared_lock otherWriteLock(other.m_WriteMutex);

		m_Data0 = other.m_Data0;
		m_Data1 = other.m_Data1;

		m_Write = &m_Data0;
		m_Read = &m_Data1;
	}

	template<typename T>
	inline ThreadPass<T>::ThreadPass(ThreadPass<T>&& other) noexcept
	{
		std::unique_lock otherReadLock(other.m_ReadMutex);
		std::unique_lock otherWriteLock(other.m_WriteMutex);

		m_Data0 = std::move(other.m_Data0);
		m_Data1 = std::move(other.m_Data1);

		m_Write = &m_Data0;
		m_Read = &m_Data1;
	}

	template<typename T>
	inline ThreadPass<T>& ThreadPass<T>::operator=(const ThreadPass<T>& other)
	{
		std::shared_lock otherReadLock(other.m_ReadMutex);
		std::shared_lock otherWriteLock(other.m_WriteMutex);
		
		m_Data0 = other.m_Data0;
		m_Data1 = other.m_Data1;

		m_Write = &m_Data0;
		m_Read = &m_Data1;

		return *this;
	}

	template<typename T>
	inline ThreadPass<T>& ThreadPass<T>::operator=(ThreadPass<T>&& other) noexcept
	{
		std::unique_lock otherReadLock(other.m_ReadMutex);
		std::unique_lock otherWriteLock(other.m_WriteMutex);
		std::unique_lock readLock(m_ReadMutex);
		std::unique_lock writeLock(m_WriteMutex);

		m_Data0 = std::move(other.m_Data0);
		m_Data1 = std::move(other.m_Data1);

		m_Write = &m_Data0;
		m_Read = &m_Data1;

		return *this;
	}

	template<typename T>
	inline ScopedLock<T> ThreadPass<T>::Write()
	{
		return ScopedLock<T>(&m_WriteMutex, *m_Write);
	}

	template<typename T>
	inline ScopedLock<T> ThreadPass<T>::Read()
	{
		return ScopedLock<T>(&m_ReadMutex, *m_Read);
	}

	template<typename T>
	inline ScopedLockRead<T> ThreadPass<T>::WriteRead() const
	{
		return ScopedLockRead<T>(&m_WriteMutex, *m_Write);
	}

	template<typename T>
	inline ScopedLockRead<T> ThreadPass<T>::ReadRead() const
	{
		return ScopedLockRead<T>(&m_ReadMutex, *m_Read);
	}

	template<typename T>
	inline void ThreadPass<T>::Swap()
	{
		std::scoped_lock lock(m_ReadMutex, m_WriteMutex);
		std::swap(m_Read, m_Write);
	}
	template<typename T>
	inline bool ThreadPass<T>::AttemptSwap()
	{
		bool swapped = false;
		if (m_ReadMutex.try_lock())
		{
			if (m_WriteMutex.try_lock())
			{
				std::swap(m_Read, m_Write);
				swapped = true;
				m_WriteMutex.unlock();
			}
			m_ReadMutex.unlock();
		}
		return swapped;
	}


	template <typename ...Args>
	class SingleThreadPass
	{
	public:
		SingleThreadPass() = default;
		SingleThreadPass(Args&& ...args);
		SingleThreadPass(const SingleThreadPass<Args...>& other);
		SingleThreadPass(SingleThreadPass<Args...>&& other) noexcept;

		SingleThreadPass& operator=(const SingleThreadPass<Args...>& other);
		SingleThreadPass& operator=(SingleThreadPass<Args...>&& other) noexcept;


		template <typename T>
		ScopedLock<T> Get();

		template <typename T, size_t Index>
		ScopedLock<T> Get();

		template <typename T>
		const ScopedLockRead<T> GetRead() const;


	private:
		std::tuple<Args...>		   m_Data;
		mutable std::shared_mutex  m_Mutex;
	};
	template<typename ...Args>
	inline SingleThreadPass<Args...>::SingleThreadPass(Args && ...args)
		:
		m_Data(std::forward<Args>(args)...)
	{
	}
	template<typename ...Args>
	inline SingleThreadPass<Args...>::SingleThreadPass(const SingleThreadPass<Args...>& other)
		:
		m_Data(other.GetRead<Args...>().As())
	{
		//std::shared_lock lock(other.m_Mutex);
		//m_Data = other.m_Data;
	}
	template<typename ...Args>
	inline SingleThreadPass<Args...>::SingleThreadPass(SingleThreadPass<Args...>&& other) noexcept
		:
		m_Data(std::move(other.Get<Args...>().As()))
	{
		//std::unique_lock lock(other.m_Mutex);
		//m_Data = std::move(other.m_Data);
	}
	template<typename ...Args>
	inline SingleThreadPass<Args...>& SingleThreadPass<Args...>::operator=(const SingleThreadPass<Args...>& other)
	{
		std::shared_lock otherLock(other.m_Mutex);
		std::unique_lock lock(m_Mutex);
		m_Data = other.m_Data;
		return *this;
	}
	template<typename ...Args>
	inline SingleThreadPass<Args...>& SingleThreadPass<Args...>::operator=(SingleThreadPass<Args...>&& other) noexcept
	{
		std::unique_lock otherLock(other.m_Mutex);
		std::unique_lock lock(m_Mutex);
		m_Data = std::move(other.m_Data);
		return *this;
	}
	template<typename ...Args>
	template<typename T>
	inline ScopedLock<T> SingleThreadPass<Args...>::Get()
	{
		return ScopedLock<T>(&m_Mutex, std::get<T>(m_Data));
	}
	
	template<typename ...Args>
	template<typename T>
	inline const ScopedLockRead<T> SingleThreadPass<Args...>::GetRead() const
	{
		return ScopedLockRead<T>(&m_Mutex, std::get<T>(m_Data));
	}

	template<typename ...Args>
	template<typename T, size_t Index>
	inline ScopedLock<T> SingleThreadPass<Args...>::Get()
	{
		return ScopedLock<T>(&m_Mutex, std::get<Index>(m_Data));
	}
}