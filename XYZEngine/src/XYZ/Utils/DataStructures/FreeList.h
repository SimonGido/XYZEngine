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
		FreeList(size_t reserve = 0)
			: m_FirstFree(-1)
		{
			if (reserve)
				m_Data.reserve(reserve);
		}

		FreeList(const FreeList<T>& other)
		{
			m_FirstFree = other.m_FirstFree;
			m_Data = other.m_Data;
		}
		
		FreeList<T>& operator=(const FreeList<T>& other)
		{
			m_FirstFree = other.m_FirstFree;
			m_Data = other.m_Data;
			return *this;
		}
		
		// Inserts an element to the free list and returns an index to it.
		int Insert(const T& elem)
		{
			if (m_FirstFree != -1)
			{
				int index = m_FirstFree;
				m_FirstFree = m_Data[m_FirstFree].Next;
				m_Data[index].Element = elem;
				return index;
			}
			else
			{
				m_Data.push_back(elem);
				return static_cast<int>(m_Data.size() - 1);
			}
		}

		template <typename... Args>
		int Emplace(Args&&... args)
		{
			if (m_FirstFree != -1)
			{
				int index = m_FirstFree;
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
		void Erase(int index)
		{
			m_Data[index].Next = m_FirstFree;
			m_FirstFree = index;
		}
		
		// Shrinks the list to the given size
		void Shrink(int size)
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
		int Range() const
		{
			return static_cast<int>(m_Data.size());
		}

		// Returns the nth element.
		T& operator[](int index)
		{
			return m_Data[index].Element;
		}

		// Returns the nth element.
		const T& operator[](int index) const
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
				: Element(other.Element)
			{}

			~FreeElement()
			{}

			FreeElement& operator =(const FreeElement& other)
			{
				Element = other.Element;
				return *this;
			}

			T Element;
			int Next;
		};


		std::vector<FreeElement> m_Data;
		int m_FirstFree;
	};
}