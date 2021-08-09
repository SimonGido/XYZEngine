#pragma once
#include <mutex>


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
	class ScopedLockReference
	{
	public:
		ScopedLockReference(std::mutex* mut, T& ref);
		~ScopedLockReference();
		ScopedLockReference(const ScopedLockReference<T>& other) = delete;
		ScopedLockReference(ScopedLockReference<T>&& other) noexcept;

		ScopedLockReference<T>& operator=(const ScopedLockReference<T>& other) = delete;
		ScopedLockReference<T>& operator=(ScopedLockReference<T>&& other) noexcept;

		T* operator->() { return &m_Ref; }
		T& As() { return m_Ref; }
		const T& As() const { return m_Ref; }

	private:
		std::mutex* m_Mutex;
		T& m_Ref;
	};


	template<typename T>
	inline ScopedLockReference<T>::ScopedLockReference(std::mutex* mut, T& ref)
		:
		m_Mutex(mut),
		m_Ref(ref)
	{
		m_Mutex->lock();
	}
	template<typename T>
	inline ScopedLockReference<T>::~ScopedLockReference()
	{
		if (m_Mutex)
			m_Mutex->unlock();
	}

	template<typename T>
	inline ScopedLockReference<T>::ScopedLockReference(ScopedLockReference<T>&& other) noexcept
		:
		m_Mutex(other.m_Mutex),
		m_Ref(other.m_Ref)
	{
		other.m_Mutex = nullptr
	}

	template<typename T>
	inline ScopedLockReference<T>& ScopedLockReference<T>::operator=(ScopedLockReference<T>&& other) noexcept
	{
		m_Mutex = other.m_Mutex;
		m_Ref   = other.m_Ref;
		other.m_Mutex = nullptr;
		return *this;
	}

	template <typename T>
	class ThreadPass
	{
	public:
		ThreadPass();

		ScopedLockReference<T> Write();
		ScopedLockReference<T> Read();

		void Swap();
		bool AttemptSwap();

	private:
		T  m_Data0;
		T  m_Data1;
		T* m_Read;
		T* m_Write;

		mutable std::mutex m_ReadMutex;
		std::mutex		   m_WriteMutex;
	};


	template<typename T>
	inline ThreadPass<T>::ThreadPass()
	{
		m_Write = &m_Data0;
		m_Read = &m_Data1;
	}

	template<typename T>
	inline ScopedLockReference<T> ThreadPass<T>::Write()
	{
		return ScopedLockReference<T>(&m_WriteMutex, *m_Write);
	}

	template<typename T>
	inline ScopedLockReference<T> ThreadPass<T>::Read()
	{
		return ScopedLockReference<T>(&m_ReadMutex, *m_Read);
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
}