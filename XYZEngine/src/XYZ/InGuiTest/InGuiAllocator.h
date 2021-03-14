#pragma once

#include "InGuiElement.h"

namespace XYZ {

	class IGPool
	{
	public:
		IGPool(const std::initializer_list<IGElementType>& types, size_t ** handles);
		~IGPool();

		template <typename T, typename ...Args>
		std::pair<T*, size_t> Allocate(Args&& ...args)
		{
			XYZ_ASSERT(m_Size + sizeof(T) <= m_Capacity, "Out of range");
			T* tmp = new((void*)&m_Data[m_Size])T(std::forward<Args>(args)...);
			std::pair<T*, size_t> result(tmp, m_Size);
			m_Size += sizeof(T);
			return result;
		}

		template <typename T>
		T* Get(size_t index)
		{
			return reinterpret_cast<T*>((void*)&m_Data[index]);
		}

		IGElement* operator[](size_t index)
		{
			return reinterpret_cast<IGElement*>((void*)&m_Data[m_Handles[index]]);
		}

		size_t Size() const
		{
			return m_Handles.size();
		}

	private:
		template <typename T>
		void destroy(size_t offset)
		{
			T* tmp = reinterpret_cast<T*>((void*)&m_Data[offset]);
			tmp->~T();
		}

	private:
		size_t m_Size;
		size_t m_Capacity;
		uint8_t* m_Data;
		std::vector<size_t> m_Handles;
		std::vector<IGElementType> m_Elements;
	};


	class IGAllocator
	{
	public:
		size_t CreatePool(const std::initializer_list<IGElementType>& types, size_t ** handles);
	
		template <typename T, typename ...Args>
		std::pair<T*, size_t> Allocate(size_t poolHandle, Args&&... args)
		{
			XYZ_ASSERT(poolHandle < m_Pools.size(), "Invalid pool handle");
			return m_Pools[poolHandle].Allocate<T, Args...>(std::forward<Args>(args)...);
		}

		template <typename T>
		T* Get(size_t poolHandle, size_t elementIndex)
		{
			XYZ_ASSERT(poolHandle < m_Pools.size(), "Invalid pool handle");
			return m_Pools[poolHandle].Get<T>(elementIndex);
		}

		std::vector<IGPool> & GetPools() { return m_Pools; }
	private:
		std::vector<IGPool> m_Pools;
	};

}