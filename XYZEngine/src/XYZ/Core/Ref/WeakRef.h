#pragma once
#include "Ref.h"

namespace XYZ {
	template<typename T>
	class WeakRef
	{
	public:
		WeakRef() = default;

		WeakRef(Ref<T> ref)
		{
			m_Instance = ref.Raw();
			if (m_Instance)
				m_RefCount = m_Instance->m_RefCount;
		}

		WeakRef(T* instance)
		{
			static_assert(std::is_base_of_v<RefCount, T>, "Type T must inherit from RefCount");
			m_Instance = instance;
			if (m_Instance)
				m_RefCount = m_Instance->m_RefCount;
		}

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		bool IsValid() const { return m_RefCount ? *m_RefCount != 0 : false; }
		
		operator bool() const { return IsValid(); }
	
		T* Raw() { return m_Instance; }
		const T* Raw() const { return m_Instance; }

		template <typename T2>
		WeakRef<T2> As() { return WeakRef<T2>((T2*)m_Instance); }

		template <typename T2>
		const WeakRef<T2> As() const { return WeakRef<T2>((T2*)m_Instance); }

	private:
		T* m_Instance = nullptr;
		std::shared_ptr<const std::atomic_uint32_t> m_RefCount;
	};
}