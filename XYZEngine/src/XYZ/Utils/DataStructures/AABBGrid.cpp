#include "stdafx.h"
#include "AABBGrid.h"

#include "XYZ/Utils/Math/Math.h"


namespace XYZ {

	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}

	AABBGrid::AABBGrid(uint32_t width, uint32_t height, uint32_t depth, float cellSize)
		:
		m_Width(width),
		m_Height(height),
		m_Depth(depth),
		m_HalfWidth(width / 2),
		m_HalfHeight(height / 2),
		m_HalfDepth(depth / 2),
		m_CellSize(cellSize),
		m_DataCount(0)
	{
		XYZ_ASSERT(m_Width % 2 == 0, "");
		XYZ_ASSERT(m_Height % 2 == 0, "");
		XYZ_ASSERT(m_Depth % 2 == 0, "");

		m_Cells.resize(m_Width * m_Height * m_Depth);
	}
	AABBGrid::AABBGrid(const AABB& aabb, float cellSize)
		:
		m_CellSize(cellSize),
		m_DataCount(0)
	{
		m_Width			= std::max(Math::RoundUp((aabb.Max.x - aabb.Min.x) / cellSize, 2), 2);
		m_Height		= std::max(Math::RoundUp((aabb.Max.y - aabb.Min.y) / cellSize, 2), 2);
		m_Depth			= std::max(Math::RoundUp((aabb.Max.z - aabb.Min.z) / cellSize, 2), 2);

		m_HalfWidth		= m_Width / 2;
		m_HalfHeight	= m_Height / 2;
		m_HalfDepth		= m_Depth / 2;
		m_Cells.resize(m_Width * m_Height * m_Depth);
	}
	void AABBGrid::Insert(const AABB& aabb, int32_t data)
	{
		const int32_t startX = aabb.Min.x / m_CellSize;
		const int32_t startY = aabb.Min.y / m_CellSize;
		const int32_t startZ = aabb.Min.z / m_CellSize;

		const int32_t endX = aabb.Max.x / m_CellSize;
		const int32_t endY = aabb.Max.y / m_CellSize;
		const int32_t endZ = aabb.Max.z / m_CellSize;

		for (int32_t x = startX + m_HalfWidth; x <= endX + m_HalfWidth; ++x)
		{
			for (int32_t y = startY + m_HalfHeight; y <= endY + m_HalfHeight; ++y)
			{
				for (int32_t z = startZ + m_HalfDepth; z <= endZ + m_HalfDepth; ++z)
				{
					const uint32_t index = Index3D(x, y, z, m_Width, m_Height);
					if (index < m_Cells.size())
					{
						m_Cells[index].Data.push_back(data);
						m_DataCount++;
					}
				}
			}
		}
	}
}