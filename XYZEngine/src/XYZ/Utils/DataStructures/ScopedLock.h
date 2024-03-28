#pragma once
#include "XYZ/Core/Core.h"

#include <mutex>
#include <shared_mutex>

namespace XYZ {
	template <typename T, typename Mutex = std::shared_mutex>
	class XYZ_API ScopedLock
	{
	public:
		ScopedLock(Mutex* mut, T& ref);
		~ScopedLock();
		ScopedLock(const ScopedLock<T, Mutex>& other) = delete;
		ScopedLock(ScopedLock<T, Mutex>&& other) noexcept;

		ScopedLock<T, Mutex>& operator=(const ScopedLock<T, Mutex>& other) = delete;
		ScopedLock<T, Mutex>& operator=(ScopedLock<T, Mutex>&& other) noexcept;

		T* operator->() { return m_Ref; }
		T& As() { return *m_Ref; }
		const T& As() const { return *m_Ref; }

	private:
		Mutex* m_Mutex;
		T*	   m_Ref;
	};


	template<typename T, typename Mutex>
	inline ScopedLock<T, Mutex>::ScopedLock(Mutex* mut, T& ref)
		:
		m_Mutex(mut),
		m_Ref(&ref)
	{
		m_Mutex->lock();
	}
	template<typename T, typename Mutex>
	inline ScopedLock<T, Mutex>::~ScopedLock()
	{
		if (m_Mutex)
			m_Mutex->unlock();
	}

	template<typename T, typename Mutex>
	inline ScopedLock<T, Mutex>::ScopedLock(ScopedLock<T, Mutex>&& other) noexcept
		:
		m_Mutex(other.m_Mutex),
		m_Ref(other.m_Ref)
	{
		other.m_Mutex = nullptr;
	}

	template<typename T, typename Mutex>
	inline ScopedLock<T, Mutex>& ScopedLock<T, Mutex>::operator=(ScopedLock<T, Mutex>&& other) noexcept
	{
		if (m_Mutex)
			m_Mutex->unlock();
		m_Mutex = other.m_Mutex;
		m_Ref = other.m_Ref;
		other.m_Mutex = nullptr;
		return *this;
	}

	template <typename T, typename Mutex = std::shared_mutex>
	class ScopedLockRead
	{
	public:
		ScopedLockRead(Mutex* mut, const T& ref);
		~ScopedLockRead();
		ScopedLockRead(const ScopedLockRead<T, Mutex>& other) = delete;
		ScopedLockRead(ScopedLockRead<T, Mutex>&& other) noexcept;

		ScopedLockRead<T, Mutex>& operator=(const ScopedLockRead<T, Mutex>& other) = delete;
		ScopedLockRead<T, Mutex>& operator=(ScopedLockRead<T, Mutex>&& other) noexcept;

		const T* operator->() { return m_Ref; }
		const T& As() const { return *m_Ref; }

	private:
		std::shared_mutex* m_Mutex;
		const T*		   m_Ref;
	};
	template<typename T, typename Mutex>
	inline ScopedLockRead<T, Mutex>::ScopedLockRead(Mutex* mut, const T& ref)
		:
		m_Mutex(mut),
		m_Ref(&ref)
	{
		m_Mutex->lock_shared();
	}
	template<typename T, typename Mutex>
	inline ScopedLockRead<T, Mutex>::~ScopedLockRead()
	{
		if (m_Mutex)
			m_Mutex->unlock_shared();
	}
	template<typename T, typename Mutex>
	inline ScopedLockRead<T, Mutex>::ScopedLockRead(ScopedLockRead<T, Mutex>&& other) noexcept
		:
		m_Mutex(other.m_Mutex),
		m_Ref(other.m_Ref)
	{
		other.m_Mutex = nullptr;
	}
	
	template<typename T, typename Mutex>
	inline ScopedLockRead<T, Mutex>& ScopedLockRead<T, Mutex>::operator=(ScopedLockRead<T, Mutex>&& other) noexcept
	{
		if (m_Mutex)
			m_Mutex->unlock_shared();
		m_Mutex = other.m_Mutex;
		m_Ref = other.m_Ref;
		other.m_Mutex = nullptr;
		return *this;
	}
}