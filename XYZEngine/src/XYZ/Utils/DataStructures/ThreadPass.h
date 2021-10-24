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
		std::shared_mutex		  m_WriteMutex;
	};


	template<typename T>
	inline ThreadPass<T>::ThreadPass()
	{
		m_Write = &m_Data0;
		m_Read = &m_Data1;
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