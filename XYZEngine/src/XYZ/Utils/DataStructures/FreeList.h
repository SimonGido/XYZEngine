#pragma once

#include <algorithm>
#include <utility>

/* Copied from
https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
*/

namespace XYZ {

	// Provides an indexed free list with constant-time removals from anywhere
	// in the list without invalidating indices. T must be trivially constructible 
	// and destructible.
	template <typename T>
	class FreeList
	{
	public:
		// Creates a new free list.
		FreeList(size_t size = 0)		
		{
			m_FirstFree = -1;
			if (size > 0)
			{
				m_Data.resize(size);
				m_FirstFree = 0;
				for (size_t i = 0; i < m_Data.size() - 1; ++i)
					m_Data[i].Next = i + 1;
			}	
		}

		FreeList(const FreeList<T>& other)
			:
			m_Data(other.m_Data),
			m_FirstFree(other.m_FirstFree)
		{
		}
		
		FreeList(FreeList<T>&& other) noexcept
			:
			m_Data(std::move(other.m_Data)),
			m_FirstFree(other.m_FirstFree)
		{
		}

		FreeList<T>& operator=(const FreeList<T>& other)
		{
			m_FirstFree = other.m_FirstFree;
			m_Data = other.m_Data;
			return *this;
		}

		FreeList<T>& operator = (FreeList<T>&& other) noexcept
		{
			m_FirstFree = other.m_FirstFree;
			m_Data = std::move(other.m_Data);
			return *this;
		}
		
		// Inserts an element to the free list and returns an index to it.
		int32_t Insert(const T& elem)
		{
			if (m_FirstFree != -1)
			{
				int32_t index = m_FirstFree;
				m_FirstFree = m_Data[m_FirstFree].Next;
				m_Data[index].Element = elem;
				return index;
			}
			else
			{
				m_Data.push_back(elem);
				return static_cast<int32_t>(m_Data.size() - 1);
			}
		}

		template <typename... Args>
		int32_t Emplace(Args&&... args)
		{
			if (m_FirstFree != -1)
			{
				int32_t index = m_FirstFree;
				m_FirstFree = m_Data[m_FirstFree].Next;
				m_Data[index].Element = T(std::forward<Args>(args)...);
				return index;
			}
			else
			{
				m_Data.emplace_back(T(std::forward<Args>(args)...));
				return static_cast<int>(m_Data.size() - 1);
			}
		}

		// Erases the nth element
		void Erase(int32_t index)
		{
			m_Data[index].Next = m_FirstFree;
			m_FirstFree = index;
		}
		
		// Shrinks the list to the given size
		void Shrink(int32_t size)
		{
			if (size <= m_FirstFree)
				m_FirstFree = -1;
			
			m_Data.resize(static_cast<size_t>(size));
		}

		// Removes all elements from the free list.
		void Clear()
		{
			m_Data.clear();
			m_FirstFree = -1;
		}

		// Returns the range of valid indices.
		int32_t Range() const
		{
			return static_cast<int32_t>(m_Data.size());
		}

		// Returns the nth element.
		T& operator[](int32_t index)
		{
			return m_Data[index].Element;
		}

	
		// Returns the nth element.
		const T& operator[](int32_t index) const
		{
			return m_Data[index].Element;
		}
	private:
		union FreeElement
		{
			FreeElement()
			{
				memset(this, 0, sizeof(FreeElement));
			}
			FreeElement(const T& el)
				: Element(el)
			{}
			FreeElement(const FreeElement& other)
				: Element(other.Element)
			{}
			FreeElement(FreeElement&& other) noexcept
				: Element(std::move(other.Element))
			{}

			~FreeElement()
			{}

			FreeElement& operator =(const FreeElement& other)
			{
				Element = other.Element;
				return *this;
			}
			operator T& () { return Element; }
			operator const T& () const { return Element; }

			T Element;
			int32_t Next;
		};


		std::vector<FreeElement> m_Data;
		int32_t m_FirstFree;
	};
}