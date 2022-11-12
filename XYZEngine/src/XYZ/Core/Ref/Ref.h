#pragma once

#include <stdint.h>
#include <atomic>
#include <functional>
#include <memory>

namespace XYZ {
	class RefCount
	{
	public:
		RefCount()
		{
			m_RefCount = std::make_shared<std::atomic_uint32_t>(0);
		}

		virtual ~RefCount() = default;

	private:
		void IncRefCount() const
		{
			(*m_RefCount)++;
		}
		void DecRefCount() const
		{
			(*m_RefCount)--;
		}

	private:
		mutable std::shared_ptr<std::atomic_uint32_t> m_RefCount;

		template<typename T>
		friend class Ref;

		template <typename T>
		friend class WeakRef;
	};
	
	
	template<typename T>
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

		bool EqualsObject(const Ref<T>& other);
		
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

		template<class T22>
		friend class Ref;
	};
	template<typename T>
	inline Ref<T>::Ref()
		: m_Instance(nullptr)
	{
	}
	template<typename T>
	inline Ref<T>::Ref(std::nullptr_t n)
		: m_Instance(nullptr)
	{
	}
	template<typename T>
	inline Ref<T>::Ref(T* instance)
		: m_Instance(instance)
	{
		static_assert(std::is_base_of<RefCount, T>::value, "Class is not RefCounted!");
		incRef();
	}

	template <typename T>
	template <typename T2>
	Ref<T>::Ref(const Ref<T2>& other)
	{
		m_Instance = (T*)other.m_Instance;
		incRef();
	}

	template<typename T>
	template<typename T2>
	inline Ref<T>::Ref(Ref<T2>&& other)
	{
		m_Instance = (T*)other.m_Instance;
		other.m_Instance = nullptr;
	}

	template <typename T>
	Ref<T>::~Ref()
	{
		decRef();
	}

	template <typename T>
	Ref<T>::Ref(const Ref<T>& other)
	: m_Instance(other.m_Instance)
	{
		incRef();
	}

	template <typename T>
	Ref<T>& Ref<T>::operator=(std::nullptr_t)
	{
		decRef();
		m_Instance = nullptr;
		return *this;
	}

	template <typename T>
	Ref<T>& Ref<T>::operator=(const Ref<T>& other)
	{
		other.incRef();
		decRef();

		m_Instance = other.m_Instance;
		return *this;
	}

	template <typename T>
	template <typename T2>
	Ref<T>& Ref<T>::operator=(const Ref<T2>& other)
	{
		other.incRef();
		decRef();

		m_Instance = other.m_Instance;
		return *this;
	}

	template <typename T>
	template <typename T2>
	Ref<T>& Ref<T>::operator=(Ref<T2>&& other)
	{
		decRef();

		m_Instance = other.m_Instance;
		other.m_Instance = nullptr;
		return *this;
	}

	template <typename T>
	T* Ref<T>::operator->()
	{
		return m_Instance;
	}

	template <typename T>
	const T* Ref<T>::operator->() const
	{
		return m_Instance;
	}

	template <typename T>
	T& Ref<T>::operator*()
	{
		return *m_Instance;
	}

	template <typename T>
	const T& Ref<T>::operator*() const
	{
		return *m_Instance;
	}

	template<typename T>
	inline bool Ref<T>::operator==(const Ref<T>& other) const
	{
		return m_Instance == other.m_Instance;
	}

	template <typename T>
	bool Ref<T>::EqualsObject(const Ref<T>& other)
	{
		if (!m_Instance || !other.m_Instance)
			return false;

		return *m_Instance == *other.m_Instance;
	}

	template <typename T>
	T* Ref<T>::Raw()
	{
		return m_Instance;
	}

	template <typename T>
	const T* Ref<T>::Raw() const
	{
		return m_Instance;
	}

	template <typename T>
	void Ref<T>::Reset(T* instance)
	{
		decRef();
		m_Instance = instance;
	}

	template <typename T>
	template <typename T2>
	Ref<T2> Ref<T>::As()
	{
		return Ref<T2>(*this);
	}
	template <typename T>
	template <typename T2>
	Ref<T2> Ref<T>::As() const
	{
		return Ref<T2>(*this);
	}

	template<typename T>
	template<typename ...Args>
	inline Ref<T> Ref<T>::Create(Args && ...args)
	{
		T* instance = new T(std::forward<Args>(args)...);
		return Ref<T>(instance);
	}


	template <typename T>
	void Ref<T>::incRef() const
	{
		if (m_Instance)
			m_Instance->IncRefCount();
	}

	template <typename T>
	void Ref<T>::decRef() const
	{
		if (m_Instance)
		{
			m_Instance->DecRefCount();
			if (*m_Instance->m_RefCount == 0)
			{
				delete m_Instance;
			}
		}
	}
}
