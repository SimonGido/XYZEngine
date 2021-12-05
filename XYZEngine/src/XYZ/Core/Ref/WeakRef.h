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
		}

		WeakRef(T* instance)
		{
			static_assert(std::is_base_of_v<RefCount, T>, "Type T must inherit from RefCount");
			m_Instance = instance;
		}

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		bool IsValid() const { return m_Instance && RefTracker::isLive(m_Instance); }
		
		operator bool() const { return IsValid(); }
	
		T* Raw() { return m_Instance; }
		const T* Raw() const { return m_Instance; }
	private:
		T* m_Instance = nullptr;
	};
}