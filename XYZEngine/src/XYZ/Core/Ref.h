#pragma once


#include <stdint.h>
#include <atomic>


namespace XYZ {

	class RefCount
	{
	public:
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

	class MemoryPool;
	class RefAllocator
	{
	public:
		static void  Init(MemoryPool* pool);
		
	private:
		static void* allocate(size_t size);
		static void  deallocate(void* handle, size_t size);

		static MemoryPool* s_Pool;
	
		template <typename T>
		friend class Ref;
	};

	
	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance(nullptr)
		{
		}
		
		Ref(std::nullptr_t n)
			: m_Instance(nullptr)
		{
		}

		Ref(T* instance)
			: m_Instance(instance)
		{
			static_assert(std::is_base_of<RefCount, T>::value, "Class is not RefCounted!");
			IncRef();
		}

		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		~Ref()
		{
			DecRef();
		}

		Ref(const Ref<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		Ref& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=(const Ref<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other)
		{
			DecRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}


		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T& Get() { return *m_Instance; }

		T* Raw() { return  m_Instance; }
		const T* Raw() const { return  m_Instance; }

		void Reset(T* instance = nullptr)
		{
			DecRef();
			m_Instance = instance;
		}

		template<typename T2>
		Ref<T2> As()
		{
			return Ref<T2>(*this);
		}

		template<typename... Args>
		static Ref<T> Create(Args&&... args)
		{
			if (RefAllocator::s_Pool)
			{
				void*  handle = RefAllocator::allocate(sizeof(T));
				return Ref<T>(new (handle)T(std::forward<Args>(args)...));
			}
			return Ref<T>(new T(std::forward<Args>(args)...));
		}
	private:
		void IncRef() const
		{
			if (m_Instance)
				m_Instance->IncRefCount();
		}

		void DecRef() const
		{
			if (m_Instance)
			{
				m_Instance->DecRefCount();
				if (m_Instance->GetRefCount() == 0)
				{
					if (RefAllocator::s_Pool)
					{
						m_Instance->~T();
						RefAllocator::deallocate((void*)m_Instance, sizeof(T));
					}
					else
						delete m_Instance;
				}
			}
		}
		T* m_Instance;

		template<class T2>
		friend class Ref;
	};

}