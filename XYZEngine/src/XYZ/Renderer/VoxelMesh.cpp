#include "stdafx.h"
#include "VoxelMesh.h"

#include "XYZ/Utils/Math/AABB.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}

	static VoxelMeshTopGrid CreateTopGridFromAABB(const AABB& aabb, float size)
	{
		VoxelMeshTopGrid topGrid;

		const uint32_t width = static_cast<uint32_t>(std::ceil(aabb.Max.x - aabb.Min.x) / size);
		const uint32_t height = static_cast<uint32_t>(std::ceil(aabb.Max.y - aabb.Min.y) / size);
		const uint32_t depth = static_cast<uint32_t>(std::ceil(aabb.Max.z - aabb.Min.z) / size);

		const glm::vec3 rayDir = glm::normalize(glm::vec3(1, 1, 1));
		const glm::vec3 delta = glm::abs(glm::vec3(width, height, depth) / rayDir);
	
		topGrid.Width = width;
		topGrid.Height = height;
		topGrid.Depth = depth;
		topGrid.Size = size;
		topGrid.Cells.resize(width * height * depth);

		return topGrid;
	}


	static std::vector<VoxelSubmesh> CompressVoxelSubmesh(const VoxelSubmesh& submesh, uint32_t scale)
	{
		std::vector<VoxelSubmesh> result;
		
		const uint32_t gridWidth = submesh.Width / scale;
		const uint32_t gridHeight = submesh.Height / scale;
		const uint32_t gridDepth = submesh.Depth / scale;

		std::vector<std::vector<uint8_t>> cellVoxels(gridWidth * gridHeight * gridDepth);
		std::vector<uint8_t> gridCells(gridWidth * gridHeight * gridDepth);


		for (uint32_t tx = 0; tx < gridWidth; tx++)
		{
			for (uint32_t ty = 0; ty < gridHeight; ty++)
			{
				for (uint32_t tz = 0; tz < gridDepth; tz++)
				{
					const uint32_t gridIndex = Index3D(tx, ty, tz, gridWidth, gridHeight);
					bool canCompress = true;
					uint32_t realX = tx * scale;
					uint32_t realY = ty * scale;
					uint32_t realZ = tz * scale;
					uint32_t index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
					uint32_t colorIndex = static_cast<uint32_t>(submesh.ColorIndices[index]);
				
					
					for (uint32_t x = 1; x < scale; x++)
					{
						for (uint32_t y = 1; y < scale; y++)
						{
							for (uint32_t z = 1; z < scale; z++)
							{
								realX = tx * scale + x;
								realY = ty * scale + y;
								realZ = tz * scale + z;
								index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
								if (colorIndex != static_cast<uint32_t>(submesh.ColorIndices[index]))
								{
									canCompress = false;
									gridCells[gridIndex] = VoxelMeshTopGrid::MultiColor;
									break;
								}
							}
						}
					}
					if (canCompress)
					{
						gridCells[gridIndex] = colorIndex;
						cellVoxels[gridIndex].push_back(static_cast<uint8_t>(colorIndex));
					}
					else
					{
						for (uint32_t x = 0; x < scale; x++)
						{
							for (uint32_t y = 0; y < scale; y++)
							{
								for (uint32_t z = 0; z < scale; z++)
								{
									realX = tx * scale + x;
									realY = ty * scale + y;
									realZ = tz * scale + z;
									index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
									colorIndex = static_cast<uint32_t>(submesh.ColorIndices[index]);
									cellVoxels[gridIndex].push_back(static_cast<uint8_t>(colorIndex));
								}
							}
						}
					}			
				}
			}
		}
		for (uint32_t tx = 0; tx < gridWidth; tx++)
		{
			for (uint32_t ty = 0; ty < gridHeight; ty++)
			{
				for (uint32_t tz = 0; tz < gridDepth; tz++)
				{
				}
			}
		}


		uint32_t index = 0;
		uint32_t voxelOffset = 0;
		for (auto cell : gridCells)
		{
			cell.VoxelOffset = voxelOffset;
			for (auto voxel : cellVoxels[index])
				grid.Voxels.push_back(voxel);

			voxelOffset += cellVoxels[index].size();
			index++;
		}

		
		return result;
	}

	static uint32_t InsertSubmeshInTopGrid(VoxelMeshTopGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh)
	{
		std::vector<std::vector<uint8_t>> cellVoxels(grid.Width * grid.Height * grid.Depth);

		const float scale = grid.Size / submesh.VoxelSize;
		const uint32_t realScale = static_cast<uint32_t>(scale);

		for (uint32_t tx = 0; tx < grid.Width; tx++)
		{
			for (uint32_t ty = 0; ty < grid.Height; ty++)
			{
				for (uint32_t tz = 0; tz < grid.Depth; tz++)
				{
					const uint32_t topgridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
					bool canCompress = true;
					uint32_t realX = tx * scale;
					uint32_t realY = ty * scale;
					uint32_t realZ = tz * scale;
					uint32_t index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
					uint32_t colorIndex = static_cast<uint32_t>(submesh.ColorIndices[index]);
					grid.Cells[topgridIndex].ColorIndex = colorIndex;
					for (uint32_t x = 1; x < realScale; x++)
					{
						for (uint32_t y = 1; y < realScale; y++)
						{
							for (uint32_t z = 1; z < realScale; z++)
							{
								realX = tx * scale + x;
								realY = ty * scale + y;
								realZ = tz * scale + z;
								index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
								if (colorIndex != static_cast<uint32_t>(submesh.ColorIndices[index]))
								{
									canCompress = false;
									grid.Cells[topgridIndex].ColorIndex = VoxelMeshTopGrid::MultiColor;
									break;
								}
							}
						}
					}
					if (canCompress)
					{
						grid.Cells[topgridIndex].ColorIndex = colorIndex;
						cellVoxels[topgridIndex].push_back(static_cast<uint8_t>(colorIndex));
					}
					else
					{
						for (uint32_t x = 0; x < realScale; x++)
						{
							for (uint32_t y = 0; y < realScale; y++)
							{
								for (uint32_t z = 0; z < realScale; z++)
								{
									realX = tx * scale + x;
									realY = ty * scale + y;
									realZ = tz * scale + z;
									index = Index3D(realX, realY, realZ, submesh.Width, submesh.Height);
									colorIndex = static_cast<uint32_t>(submesh.ColorIndices[index]);
									cellVoxels[topgridIndex].push_back(static_cast<uint8_t>(colorIndex));
								}
							}
						}
					}
				}
			}
		}

		/*
		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					uint32_t tx = x / scale;
					uint32_t ty = y / scale;
					uint32_t tz = z / scale;


					const uint32_t topgridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
					if (topgridIndex >= grid.Cells.size())
						continue;

					if (!cellsInitialized[topgridIndex])
					{
						grid.Cells[topgridIndex].ColorIndex = submesh.ColorIndices[index];
						cellsInitialized[topgridIndex] = true;
						cellVoxels[topgridIndex].push_back(submesh.ColorIndices[index]);
					}

					// Cell already has stored color, if it does not equal with another voxel color it is multi color
					if (grid.Cells[topgridIndex].ColorIndex != submesh.ColorIndices[index])
					{
						grid.Cells[topgridIndex].ColorIndex = VoxelMeshTopGrid::MultiColor;
					}
					// It is multi color, no compression happens
					if (grid.Cells[topgridIndex].ColorIndex == VoxelMeshTopGrid::MultiColor)
					{
						cellVoxels[topgridIndex].push_back(submesh.ColorIndices[index]);
					}


					const glm::vec3 voxelPosition = {
						x * submesh.VoxelSize,
						y * submesh.VoxelSize,
						z * submesh.VoxelSize
					};
					const glm::ivec3 topGridCellStart = {
						std::floor(voxelPosition.x / grid.Size),
						std::floor(voxelPosition.y / grid.Size),
						std::floor(voxelPosition.z / grid.Size)
					};
					glm::ivec3 topGridCellEnd = {
						std::floor((voxelPosition.x + submesh.VoxelSize) / grid.Size),
						std::floor((voxelPosition.y + submesh.VoxelSize) / grid.Size),
						std::floor((voxelPosition.z + submesh.VoxelSize) / grid.Size)
					};
					topGridCellEnd = topGridCellStart;
					//for (uint32_t tx = topGridCellStart.x; tx <= topGridCellEnd.x; tx++)
					//{
					//	for (uint32_t ty = topGridCellStart.y; ty <= topGridCellEnd.y; ty++)
					//	{
					//		for (uint32_t tz = topGridCellStart.z; tz <= topGridCellEnd.z; tz++)
					//		{
					//			const uint32_t topgridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
					//			if (topgridIndex >= grid.Cells.size())
					//				continue;
					//
					//			if (!cellsInitialized[topgridIndex])
					//			{
					//				grid.Cells[topgridIndex].ColorIndex = submesh.ColorIndices[index];
					//				cellsInitialized[topgridIndex] = true;
					//				cellVoxels[topgridIndex].push_back(submesh.ColorIndices[index]);
					//			}
					//
					//			// Cell already has stored color, if it does not equal with another voxel color it is multi color
					//			if (grid.Cells[topgridIndex].ColorIndex != submesh.ColorIndices[index])
					//			{
					//				grid.Cells[topgridIndex].ColorIndex = VoxelMeshTopGrid::MultiColor;
					//			}
					//			// It is multi color, no compression happens
					//			if (grid.Cells[topgridIndex].ColorIndex == VoxelMeshTopGrid::MultiColor)
					//			{
					//				cellVoxels[topgridIndex].push_back(submesh.ColorIndices[index]);
					//			}
					//		}
					//	}
					//}
					
				}
			}
		}
		*/
		uint32_t index = 0;
		uint32_t voxelOffset = 0;
		for (auto &cell : grid.Cells)
		{
			cell.VoxelOffset = voxelOffset;
			for (auto voxel : cellVoxels[index])
				grid.Voxels.push_back(voxel);

			voxelOffset += cellVoxels[index].size();
			index++;
		}
		uint32_t savedSpace = submesh.ColorIndices.size() - grid.Voxels.size();
		return savedSpace;
	}


	VoxelSourceMesh::VoxelSourceMesh(const Ref<VoxelMeshSource>& meshSource)
		:
		m_MeshSource(meshSource)
	{
	}
	VoxelSourceMesh::VoxelSourceMesh(const AssetHandle& meshSourceHandle)
		:
		m_MeshSource(meshSourceHandle)
	{
	}
	const std::vector<VoxelSubmesh>& VoxelSourceMesh::GetSubmeshes() const
	{
		return m_MeshSource->GetSubmeshes();
	}
	const std::vector<VoxelInstance>& VoxelSourceMesh::GetInstances() const
	{
		return m_MeshSource->GetInstances();
	}
	const AssetHandle& VoxelSourceMesh::GetRenderID() const
	{
		return GetHandle();
	}
	bool VoxelSourceMesh::NeedUpdate() const
	{
		return false;
	}

	VoxelProceduralMesh::VoxelProceduralMesh()
		:
		m_NumVoxels(0),
		m_Dirty(false),
		m_HasTopGrid(false),
		m_GeneratingTopGrid(false)
	{
	}

	void VoxelProceduralMesh::SetSubmeshes(const std::vector<VoxelSubmesh>& submeshes)
	{
		m_Submeshes = submeshes;
		m_NumVoxels = 0;
		uint32_t index = 0;
		for (auto& submesh : m_Submeshes)
		{
			m_NumVoxels += static_cast<uint32_t>(submesh.ColorIndices.size());
		}
		m_Dirty = true;
		m_DirtySubmeshes.clear();
	}

	void VoxelProceduralMesh::SetInstances(const std::vector<VoxelInstance>& instances)
	{
		m_Instances = instances;
		m_Dirty = true;
		m_DirtySubmeshes.clear();
	}

	void VoxelProceduralMesh::SetColorPallete(const std::array<VoxelColor, 256>& pallete)
	{
		m_ColorPallete = pallete;
	}

	void VoxelProceduralMesh::GenerateTopGridAsync(float size)
	{
		if (!m_GeneratingTopGrid)
		{
			m_HasTopGrid = false;
			m_GeneratingTopGrid = true;
			Ref<VoxelProceduralMesh> instance = this;
			auto& pool = Application::Get().GetThreadPool();
			pool.PushJob([instance, submeshes = m_Submeshes, colorPallete = m_ColorPallete, size]() mutable {
				instance->m_TopGrid = generateTopGrid(submeshes, colorPallete, size);
				instance->m_HasTopGrid = true;
				instance->m_GeneratingTopGrid = false;
				instance->m_Dirty = true;
			});
		}
	}

	void VoxelProceduralMesh::SetVoxelColor(uint32_t submeshIndex, uint32_t x, uint32_t y, uint32_t z, uint8_t value)
	{
		auto& submesh = m_Submeshes[submeshIndex];
		const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
		submesh.ColorIndices[index] = value;
		auto& range = m_DirtySubmeshes[submeshIndex];
		range.Start = std::min(range.Start, index);
		range.End = std::max(range.End, index + 1);
	}

	const std::array<VoxelColor, 256>& VoxelProceduralMesh::GetColorPallete() const
	{
		return m_ColorPallete;
	}

	const std::vector<VoxelSubmesh>& VoxelProceduralMesh::GetSubmeshes() const
	{
		return m_Submeshes;
	}
	const std::vector<VoxelInstance>& VoxelProceduralMesh::GetInstances() const
	{
		return m_Instances;
	}
	const AssetHandle& VoxelProceduralMesh::GetRenderID() const
	{
		return GetHandle();
	}
	bool VoxelProceduralMesh::NeedUpdate() const
	{
		bool dirty = m_Dirty;
		m_Dirty = false;
		return dirty;
	}
	bool VoxelProceduralMesh::HasTopGrid() const
	{
		return m_HasTopGrid;
	}
	std::unordered_map<uint32_t, VoxelMesh::DirtyRange> VoxelProceduralMesh::DirtySubmeshes() const
	{
		return std::move(m_DirtySubmeshes);
	}
	VoxelMeshTopGrid VoxelProceduralMesh::generateTopGrid(const std::vector<VoxelSubmesh>& submeshes, const std::array<VoxelColor, 256>& colorPallete, float size)
	{
		AABB aabb(glm::vec3(0), glm::vec3(FLT_MIN));
		for (auto& submesh : submeshes)
		{
			aabb.Max.x = std::max(static_cast<float>(submesh.Width) * submesh.VoxelSize, aabb.Max.x);
			aabb.Max.y = std::max(static_cast<float>(submesh.Height) * submesh.VoxelSize, aabb.Max.y);
			aabb.Max.z = std::max(static_cast<float>(submesh.Depth) * submesh.VoxelSize, aabb.Max.z);
		}
		VoxelMeshTopGrid topGrid = CreateTopGridFromAABB(aabb, size);
		for (auto& submesh : submeshes)
		{
			InsertSubmeshInTopGrid(topGrid, colorPallete, submesh);
		}
		return topGrid;
	}
}