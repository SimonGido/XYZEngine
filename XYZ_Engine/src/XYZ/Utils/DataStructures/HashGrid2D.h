#pragma once

#include <glm/glm.hpp>

namespace XYZ {
/*! @class HashGrid2D
*	@brief 2D HashGrid for storing elements
*/
	template<typename T>
	class HashGrid2D
	{
	public:
		HashGrid2D(int cellSize, int tableSize)
			: m_CellSize(cellSize), m_TableSize(tableSize)
		{
			m_Table.resize(m_TableSize);
		}
		
		/** Inserts an element into the hashgrid */
		void Insert(const T& element, const glm::vec2& pos, const glm::vec2& size)
		{
			for (int i = (int)pos.x; i < int(pos.x + size.x); ++i)
			{
				for (int j = (int)pos.y; j < int(pos.y + size.y); ++j)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					m_Table[index].elements.push_back(element);
				}
			}
		}

		/** Removes an element from the hashgrid */
		void Remove(const T& element, const glm::vec2& pos, const glm::vec2& size)
		{
			for (int i = (int)pos.x; i < int(pos.x + size.x); ++i)
			{
				for (int j = (int)pos.y; j < int(pos.y + size.y); ++j)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					auto it = std::find(m_Table[index].elements.begin(), m_Table[index].elements.end(), element);
					if (it != m_Table[index].elements.end())
						m_Table[index].elements.erase(it);
				}
			}
		}
		
		/** Returns the element count */
		size_t GetElements(T** buffer, const glm::vec2& pos, const glm::vec2& size)
		{
			std::vector<size_t> indices;
			size_t count = 0;
			for (int i = (int)pos.x; i < int(pos.x + size.x); ++i)
			{
				for (int j = (int)pos.y; j < int(pos.y + size.y); ++j)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					count += m_Table[index].elements.size();
					indices.push_back(index);
				}
			}

			*buffer = new T[count * sizeof(int)];
			int* ptr = *buffer;
			for (auto it : indices)
			{
				size_t elementsSize = m_Table[it].elements.size();
				memcpy(ptr, m_Table[it].elements.data(), elementsSize * sizeof(T));
				ptr += elementsSize;
			}

			return count;
		}

	private:
		struct Cell
		{
			std::vector<T> elements;
		};


		int m_CellSize;
		int m_TableSize;

		std::vector<Cell> m_Table;
	};
}