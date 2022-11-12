#pragma once
#include "XYZ/Core/Core.h"

#include <algorithm>
#include <utility>
#include <variant>

namespace XYZ {

	template <typename T>
	class XYZ_API FreeListIterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

	public:
		FreeListIterator(T* ptr = nullptr) { m_ptr = ptr; }
		FreeListIterator(const FreeListIterator<T>& rawIterator) = default;
		~FreeListIterator() {}

		FreeListIterator<T>& operator=(const FreeListIterator<T>&rawIterator) = default;
		FreeListIterator<T>& operator=(T * ptr) { m_ptr = ptr; return (*this); }

		operator bool() const {return m_ptr != null; }

		bool                 operator==(const FreeListIterator<T>&rawIterator) const { return (m_ptr == rawIterator.getConstPtr()); }
		bool                 operator!=(const FreeListIterator<T>&rawIterator) const { return (m_ptr != rawIterator.getConstPtr()); }

		FreeListIterator<T>& operator+=(const difference_type & movement) { m_ptr += movement; return (*this); }
		FreeListIterator<T>& operator-=(const difference_type & movement) { m_ptr -= movement; return (*this); }
		FreeListIterator<T>& operator++() { ++m_ptr; return (*this); }
		FreeListIterator<T>& operator--() { --m_ptr; return (*this); }
		FreeListIterator<T>                   operator++(int) { auto temp(*this); ++m_ptr; return temp; }
		FreeListIterator<T>                   operator--(int) { auto temp(*this); --m_ptr; return temp; }
		FreeListIterator<T>                   operator+(const difference_type & movement) { auto oldPtr = m_ptr; m_ptr += movement; auto temp(*this); m_ptr = oldPtr; return temp; }
		FreeListIterator<T>                   operator-(const difference_type & movement) { auto oldPtr = m_ptr; m_ptr -= movement; auto temp(*this); m_ptr = oldPtr; return temp; }

		difference_type                             operator-(const FreeListIterator<T>&rawIterator) { return std::distance(rawIterator.getPtr(), this->getPtr()); }

		T& operator*() { return *m_ptr; }
		const T& operator*()const { return *m_ptr; }
		T* operator->() { return m_ptr; }

		T* getPtr()const { return m_ptr; }
		const T* getConstPtr()const { return m_ptr; }

	protected:
		T* m_ptr;
	};

	template <typename T>
	class XYZ_API FreeList
	{
	public:
		using iterator = FreeListIterator<T>;
		using const_iterator = FreeListIterator<T>;

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