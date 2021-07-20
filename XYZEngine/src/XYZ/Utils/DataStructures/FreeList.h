#pragma once

#include <algorithm>
#include <utility>
#include <variant>

namespace XYZ {

	template <typename T>
	class FreeList
	{
	public:
		FreeList(int32_t size = 0);
		FreeList(const FreeList<T>& other);
		FreeList(FreeList<T>&& other) noexcept;
			
		FreeList<T>& operator=(const FreeList<T>& other);
		FreeList<T>& operator =(FreeList<T>&& other) noexcept;
		
		
		int32_t Insert(const T& elem);

		template <typename... Args>
		int32_t Emplace(Args&&... args);
			
		void    Erase(int32_t index);
		void    Clear();

		bool    Valid(int32_t index) const;
		int32_t Range() const;
		int32_t Next() const;
		
		T&	     operator[](int32_t index);
		const T& operator[](int32_t index) const;
	private:
		std::vector<std::variant<int32_t, T>> m_Data;
		int32_t m_FirstFree;
	};
	template<typename T>
	inline FreeList<T>::FreeList(int32_t size)
	{
		static_assert(!std::is_same<T, int32_t>::value, "FreeList can not store int32_t");
		if (size > 0)
		{
			m_Data.resize(size);
			m_FirstFree = 0;
			for (int32_t i = 0; i < size - 1; ++i)
				m_Data[i] = i + 1;
		}
		else
			m_FirstFree = -1;
	}
	template<typename T>
	inline FreeList<T>::FreeList(const FreeList<T>& other)
		:
		m_FirstFree(other.m_FirstFree),
		m_Data(other.m_Data)
	{
	}
	template<typename T>
	inline FreeList<T>::FreeList(FreeList<T>&& other) noexcept
		:
		m_Data(std::move(other.m_Data)),
		m_FirstFree(other.m_FirstFree)
	{}
	template<typename T>
	inline FreeList<T>& FreeList<T>::operator=(const FreeList<T>& other)
	{
		m_FirstFree = other.m_FirstFree;
		m_Data = other.m_Data;
		return *this;
	}
	template<typename T>
	inline FreeList<T>& FreeList<T>::operator=(FreeList<T>&& other) noexcept
	{
		m_FirstFree = other.m_FirstFree;
		m_Data = std::move(other.m_Data);
		return *this;
	}
	template<typename T>
	inline int32_t FreeList<T>::Insert(const T& elem)
	{
		if (m_FirstFree != -1)
		{
			int32_t index = m_FirstFree;
			m_FirstFree = std::get<int32_t>(m_Data[m_FirstFree]);
			m_Data[index] = elem;
			return index;
		}
		else
		{
			m_Data.push_back(elem);
			return static_cast<int32_t>(m_Data.size() - 1);
		}
	}
	template<typename T>
	template<typename ...Args>
	inline int32_t FreeList<T>::Emplace(Args && ...args)
	{
		if (m_FirstFree != -1)
		{
			int32_t index = m_FirstFree;
			m_FirstFree = std::get<int32_t>(m_Data[m_FirstFree]);
			m_Data[index] = T(std::forward<Args>(args)...);
			return index;
		}
		else
		{
			m_Data.emplace_back(T(std::forward<Args>(args)...));
			return static_cast<int>(m_Data.size() - 1);
		}
	}

	template<typename T>
	inline void FreeList<T>::Erase(int32_t index)
	{
		m_Data[index] = m_FirstFree;
		m_FirstFree = index;
	}

	template<typename T>
	inline void FreeList<T>::Clear()
	{
		m_Data.clear();
		m_FirstFree = -1;
	}
	template<typename T>
	inline bool FreeList<T>::Valid(int32_t index) const
	{
		return !std::holds_alternative<int32_t>(m_Data[index]);
	}
	template<typename T>
	inline int32_t FreeList<T>::Range() const
	{
		return static_cast<int32_t>(m_Data.size());
	}
	template<typename T>
	inline int32_t FreeList<T>::Next() const
	{
		if (m_FirstFree == -1)
			return (int32_t)m_Data.size();
		return m_FirstFree;
	}
	template<typename T>
	inline T& FreeList<T>::operator[](int32_t index)
	{
		return std::get<T>(m_Data[index]);
	}
	template<typename T>
	inline const T& FreeList<T>::operator[](int32_t index) const
	{
		return std::get<T>(m_Data[index]);
	}
	
}