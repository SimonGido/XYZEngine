#pragma once
#include "RefTracker.h"
#include "RefAllocator.h"

#include <stdint.h>
#include <atomic>
#include <functional>

namespace XYZ {
	class RefCount
	{
	public:
		virtual ~RefCount() = default;

		void IncRefCount() const
		{
			m_RefCount++;
		}
		void DecRefCount() const
		{
			m_RefCount--;
		}

		uint32_t GetRefCount() const { return m_RefCount; }
	private:
		mutable std::atomic<uint32_t> m_RefCount = 0;

	};	
	
	
	template<typename T, typename Allocator = RefAllocator>
	class Ref
	{
	public:
		Ref();
		Ref(std::nullptr_t n);
		Ref(T* instance);
		~Ref();	

		template<typename T2>
		Ref(const Ref<T2>& other);
		
		template<typename T2>
		Ref(Ref<T2>&& other);

		
		Ref(const Ref<T>& other);
		
		Ref& operator=(std::nullptr_t);

		
		Ref& operator=(const Ref<T>& other);

		
		template<typename T2>
		Ref& operator=(const Ref<T2>& other);

		
		template<typename T2>
		Ref& operator=(Ref<T2>&& other);
	

		T* operator->();
		const T* operator->() const;

		T& operator*();
		const T& operator*() const;
		
		bool operator==(const Ref<T>& other) const;

		bool EqualsObject(const Ref<T, Allocator>& other);
		
		T* Raw();
		const T* Raw() const;

		void Reset(T* instance = nullptr);

		template<typename T2>
		Ref<T2> As();

		template<typename T2>
		Ref<T2> As() const;

		template<typename... Args>
		static Ref<T> Create(Args&&... args);
	private:
		void incRef() const;
		
		void decRef() const;
		
		T*		  m_Instance;

		template<class T2, typename Allocator2>
		friend class Ref;
	};
	template<typename T, typename Allocator>
	inline Ref<T, Allocator>::Ref()
		: m_Instance(nullptr)
	{
	}
	template<typename T, typename Allocator>
	inline Ref<T, Allocator>::Ref(std::nullptr_t n)
		: m_Instance(nullptr)
	{
	}
	template<typename T, typename Allocator>
	inline Ref<T, Allocator>::Ref(T* instance)
		: m_Instance(instance)
	{
		static_assert(std::is_base_of<RefCount, T>::value, "Class is not RefCounted!");
		incRef();
	}

	template <typename T, typename Allocator>
	template <typename T2>
	Ref<T, Allocator>::Ref(const Ref<T2>& other)
	{
		m_Instance = (T*)other.m_Instance;
		incRef();
	}

	template<typename T, typename Allocator>
	template<typename T2>
	inline Ref<T, Allocator>::Ref(Ref<T2>&& other)
	{
		m_Instance = (T*)other.m_Instance;
		other.m_Instance = nullptr;
	}

	template <typename T, typename Allocator>
	Ref<T, Allocator>::~Ref()
	{
		decRef();
	}

	template <typename T, typename Allocator>
	Ref<T, Allocator>::Ref(const Ref<T>& other)
	: m_Instance(other.m_Instance)
	{
		incRef();
	}

	template <typename T, typename Allocator>
	Ref<T, Allocator>& Ref<T, Allocator>::operator=(std::nullptr_t)
	{
		decRef();
		m_Instance = nullptr;
		return *this;
	}

	template <typename T, typename Allocator>
	Ref<T, Allocator>& Ref<T, Allocator>::operator=(const Ref<T>& other)
	{
		other.incRef();
		decRef();

		m_Instance = other.m_Instance;
		return *this;
	}

	template <typename T, typename Allocator>
	template <typename T2>
	Ref<T, Allocator>& Ref<T, Allocator>::operator=(const Ref<T2>& other)
	{
		other.incRef();
		decRef();

		m_Instance = other.m_Instance;
		return *this;
	}

	template <typename T, typename Allocator>
	template <typename T2>
	Ref<T, Allocator>& Ref<T, Allocator>::operator=(Ref<T2>&& other)
	{
		decRef();

		m_Instance = other.m_Instance;
		other.m_Instance = nullptr;
		return *this;
	}

	template <typename T, typename Allocator>
	T* Ref<T, Allocator>::operator->()
	{
		return m_Instance;
	}

	template <typename T, typename Allocator>
	const T* Ref<T, Allocator>::operator->() const
	{
		return m_Instance;
	}

	template <typename T, typename Allocator>
	T& Ref<T, Allocator>::operator*()
	{
		return *m_Instance;
	}

	template <typename T, typename Allocator>
	const T& Ref<T, Allocator>::operator*() const
	{
		return *m_Instance;
	}

	template<typename T, typename Allocator>
	inline bool Ref<T, Allocator>::operator==(const Ref<T>& other) const
	{
		return m_Instance == other.m_Instance;
	}

	template <typename T, typename Allocator>
	bool Ref<T, Allocator>::EqualsObject(const Ref<T, Allocator>& other)
	{
		if (!m_Instance || !other.m_Instance)
			return false;

		return *m_Instance == *other.m_Instance;
	}

	template <typename T, typename Allocator>
	T* Ref<T, Allocator>::Raw()
	{
		return m_Instance;
	}

	template <typename T, typename Allocator>
	const T* Ref<T, Allocator>::Raw() const
	{
		return m_Instance;
	}

	template <typename T, typename Allocator>
	void Ref<T, Allocator>::Reset(T* instance)
	{
		decRef();
		m_Instance = instance;
	}

	template <typename T, typename Allocator>
	template <typename T2>
	Ref<T2> Ref<T, Allocator>::As()
	{
		return Ref<T2, Allocator>(*this);
	}
	template <typename T, typename Allocator>
	template <typename T2>
	Ref<T2> Ref<T, Allocator>::As() const
	{
		return Ref<T2, Allocator>(*this);
	}

	template<typename T, typename Allocator>
	template<typename ...Args>
	inline Ref<T> Ref<T, Allocator>::Create(Args && ...args)
	{
		Allocator allocator{};
		void* ptr = allocator.Allocate(sizeof(T));
		RefTracker::addToLiveReferences(ptr);
		new (ptr)T(std::forward<Args>(args)...);
		return Ref<T, Allocator>(static_cast<T*>(ptr));
	}


	template <typename T, typename Allocator>
	void Ref<T, Allocator>::incRef() const
	{
		if (m_Instance)
			m_Instance->IncRefCount();
	}

	template <typename T, typename Allocator>
	void Ref<T, Allocator>::decRef() const
	{
		if (m_Instance)
		{
			m_Instance->DecRefCount();
			if (m_Instance->GetRefCount() == 0)
			{
				RefTracker::removeFromLiveReferences((void*)(m_Instance));
				m_Instance->~T();
				Allocator allocator;
				allocator.Deallocate(m_Instance);
			}
		}
	}
}
