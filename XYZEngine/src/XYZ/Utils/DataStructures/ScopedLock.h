#pragma once
#include <mutex>
#include <shared_mutex>

namespace XYZ {
	template <typename T>
	class ScopedLock
	{
	public:
		ScopedLock(std::shared_mutex* mut, T& ref);
		~ScopedLock();
		ScopedLock(const ScopedLock<T>& other) = delete;
		ScopedLock(ScopedLock<T>&& other) noexcept;

		ScopedLock<T>& operator=(const ScopedLock<T>& other) = delete;
		ScopedLock<T>& operator=(ScopedLock<T>&& other) noexcept;

		T* operator->() { return m_Ref; }
		T& As() { return *m_Ref; }
		const T& As() const { return *m_Ref; }

	private:
		std::shared_mutex* m_Mutex;
		T*				   m_Ref;
	};


	template<typename T>
	inline ScopedLock<T>::ScopedLock(std::shared_mutex* mut, T& ref)
		:
		m_Mutex(mut),
		m_Ref(&ref)
	{
		m_Mutex->lock();
	}
	template<typename T>
	inline ScopedLock<T>::~ScopedLock()
	{
		if (m_Mutex)
			m_Mutex->unlock();
	}

	template<typename T>
	inline ScopedLock<T>::ScopedLock(ScopedLock<T>&& other) noexcept
		:
		m_Mutex(other.m_Mutex),
		m_Ref(other.m_Ref)
	{
		other.m_Mutex = nullptr;
	}

	template<typename T>
	inline ScopedLock<T>& ScopedLock<T>::operator=(ScopedLock<T>&& other) noexcept
	{
		if (m_Mutex)
			m_Mutex->unlock();
		m_Mutex = other.m_Mutex;
		m_Ref = other.m_Ref;
		other.m_Mutex = nullptr;
		return *this;
	}

	template <typename T>
	class ScopedLockRead
	{
	public:
		ScopedLockRead(std::shared_mutex* mut, const T& ref);
		~ScopedLockRead();
		ScopedLockRead(const ScopedLockRead<T>& other) = delete;
		ScopedLockRead(ScopedLockRead<T>&& other) noexcept;

		ScopedLockRead<T>& operator=(const ScopedLockRead<T>& other) = delete;
		ScopedLockRead<T>& operator=(ScopedLockRead<T>&& other) noexcept;

		const T* operator->() { return m_Ref; }
		const T& As() const { return *m_Ref; }

	private:
		std::shared_mutex* m_Mutex;
		const T*		   m_Ref;
	};
	template<typename T>
	inline ScopedLockRead<T>::ScopedLockRead(std::shared_mutex* mut, const T& ref)
		:
		m_Mutex(mut),
		m_Ref(&ref)
	{
		m_Mutex->lock_shared();
	}
	template<typename T>
	inline ScopedLockRead<T>::~ScopedLockRead()
	{
		if (m_Mutex)
			m_Mutex->unlock_shared();
	}
	template<typename T>
	inline ScopedLockRead<T>::ScopedLockRead(ScopedLockRead<T>&& other) noexcept
		:
		m_Mutex(other.m_Mutex),
		m_Ref(other.m_Ref)
	{
		other.m_Mutex = nullptr;
	}
	
	template<typename T>
	inline ScopedLockRead<T>& ScopedLockRead<T>::operator=(ScopedLockRead<T>&& other) noexcept
	{
		if (m_Mutex)
			m_Mutex->unlock_shared();
		m_Mutex = other.m_Mutex;
		m_Ref = other.m_Ref;
		other.m_Mutex = nullptr;
		return *this;
	}
}