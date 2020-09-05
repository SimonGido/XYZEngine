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
		HashGrid2D(size_t cellSize, size_t tableSize)
			: m_CellSize(cellSize), m_TableSize(tableSize)
		{
			m_Table.resize(m_TableSize);
		}
		
		/** Inserts an element size_to the hashgrid */
		void Insert(const T& element, const glm::vec2& pos, const glm::vec2& size)
		{
			size_t startX = (size_t)std::floor(abs(pos.x));
			size_t startY = (size_t)std::floor(abs(pos.y));
			size_t endX = (size_t)std::ceil(abs(pos.x)) + (size_t)std::ceil(abs(size.x));
			size_t endY = (size_t)std::ceil(abs(pos.y)) + (size_t)std::ceil(abs(size.y));

			for (size_t i = startX; i < endX; i += m_CellSize)
			{
				for (size_t j = startY; j < endY; j += m_CellSize)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					m_Table[index].elements.push_back(element);
				}
			}
		}

		/** Removes an element from the hashgrid */
		bool Remove(const T& element, const glm::vec2& pos, const glm::vec2& size)
		{
			size_t startX = (size_t)std::floor(abs(pos.x));
			size_t startY = (size_t)std::floor(abs(pos.y));
			size_t endX = (size_t)std::ceil(abs(pos.x)) + (size_t)std::ceil(abs(size.x));
			size_t endY = (size_t)std::ceil(abs(pos.y)) + (size_t)std::ceil(abs(size.y));

			bool removed = false;
			for (size_t i = startX; i < endX; i += m_CellSize)
			{
				for (size_t j = startY; j < endY; j += m_CellSize)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					auto it = std::find(m_Table[index].elements.begin(), m_Table[index].elements.end(), element);
					if (it != m_Table[index].elements.end())
					{
						m_Table[index].elements.erase(it);
						removed = true;
					}
				}
			}
			return removed;
		}
		
		/** Returns the element count */
		size_t GetElements(T** buffer, const glm::vec2& pos, const glm::vec2& size)
		{
			std::vector<size_t> indices;
			size_t count = 0;
			size_t startX = (size_t)std::floor(abs(pos.x));
			size_t startY = (size_t)std::floor(abs(pos.y));
			size_t endX = (size_t)std::ceil(abs(pos.x)) + (size_t)std::ceil(abs(size.x));
			size_t endY = (size_t)std::ceil(abs(pos.y)) + (size_t)std::ceil(abs(size.y));

			for (size_t i = startX; i < endX; i += m_CellSize)
			{
				for (size_t j = startY; j < endY; j += m_CellSize)
				{
					size_t index = ((size_t)floor(i / m_CellSize) + (size_t)floor(j / m_CellSize)) % m_TableSize;
					count += m_Table[index].elements.size();
					indices.push_back(index);
				}
			}

			if (count)
			{
				*buffer = new T[count * sizeof(T)];
				T* ptr = *buffer;
				for (auto it : indices)
				{
					size_t elementsCount = m_Table[it].elements.size();
					memcpy(ptr, m_Table[it].elements.data(), elementsCount * sizeof(T));
					ptr += elementsCount;
				}
			}
			return count;
		}

		void Clear()
		{
			for (auto& cell : m_Table)
			{
				cell.elements.clear();
			}
		}
	private:
		struct Cell
		{
			std::vector<T> elements;
		};


		size_t m_CellSize;
		size_t m_TableSize;

		std::vector<Cell> m_Table;
	};
}