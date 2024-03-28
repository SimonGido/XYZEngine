#pragma once
#include "XYZ/Utils/Math/AABB.h"

namespace XYZ {

	class AABBGrid
	{
	public:
		struct Cell
		{
			std::vector<int32_t> Data;
		};

		AABBGrid(uint32_t width, uint32_t height, uint32_t depth, float cellSize);
		AABBGrid(const AABB& aabb, float cellSize);

		void Insert(const AABB& aabb, int32_t data);

		const std::vector<Cell>& GetCells() const { return m_Cells; }

		uint32_t GetDataCount() const { return m_DataCount; }
	private:
		
	private:
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Depth;
		uint32_t m_HalfWidth;
		uint32_t m_HalfHeight;
		uint32_t m_HalfDepth;

		float	 m_CellSize; 

		std::vector<Cell> m_Cells;
		uint32_t m_DataCount;
	};

}