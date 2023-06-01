#include "stdafx.h"
#include "VoxelMesh.h"

#include "XYZ/Utils/Math/AABB.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}


	static VoxelMeshAccelerationGrid CreateAccelerationGridFromAABB(const AABB& aabb, float size)
	{
		VoxelMeshAccelerationGrid AccelerationGrid;

		const uint32_t width = static_cast<uint32_t>(std::ceil(aabb.Max.x - aabb.Min.x) / size);
		const uint32_t height = static_cast<uint32_t>(std::ceil(aabb.Max.y - aabb.Min.y) / size);
		const uint32_t depth = static_cast<uint32_t>(std::ceil(aabb.Max.z - aabb.Min.z) / size);

		AccelerationGrid.Width = width;
		AccelerationGrid.Height = height;
		AccelerationGrid.Depth = depth;
		AccelerationGrid.Size = size;
		AccelerationGrid.Cells.resize(width * height * depth, 0);
		
		return AccelerationGrid;
	}

	static void FillAccelerationGrid(VoxelMeshAccelerationGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh, const glm::uvec3& cellStart, const glm::uvec3& cellEnd, uint32_t cellSize)
	{
		for (uint32_t ax = cellStart.x; ax < cellEnd.x; ++ax)
		{
			for (uint32_t ay = cellStart.y; ay < cellEnd.y; ++ay)
			{
				for (uint32_t az = cellStart.z; az < cellEnd.z; ++az)
				{
					const uint32_t acIndex = Index3D(ax, ay, az, grid.Width, grid.Height);
					if (acIndex >= grid.Cells.size())
						continue;

					for (uint32_t x = 0; x < cellSize; ++x)
					{
						for (uint32_t y = 0; y < cellSize; ++y)
						{
							for (uint32_t z = 0; z < cellSize; ++z)
							{
								const uint32_t index = Index3D(ax * cellSize + x, ay * cellSize + y, az * cellSize + z, submesh.Width, submesh.Height);
								if (index < submesh.ColorIndices.size())
								{
									const auto& color = colorPallete[submesh.ColorIndices[index]];
									if (color.A != 0)
										grid.Cells[acIndex]++;
								}
							}
						}
					}
				}
			}
		}
	}


	static void FillAccelerationGrid(VoxelMeshAccelerationGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh,  uint32_t scale)
	{
		for (uint32_t ax = 0; ax < grid.Width; ++ax)
		{
			for (uint32_t ay = 0; ay < grid.Height; ++ay)
			{
				for (uint32_t az = 0; az < grid.Depth; ++az)
				{
					const uint32_t acIndex = Index3D(ax, ay, az, grid.Width, grid.Height);
					for (uint32_t x = 0; x < scale; ++x)
					{
						for (uint32_t y = 0; y < scale; ++y)
						{
							for (uint32_t z = 0; z < scale; ++z)
							{
								const uint32_t index = Index3D(ax * scale + x, ay * scale + y, az * scale + z, submesh.Width, submesh.Height);
								if (index < submesh.ColorIndices.size())
								{
									const auto& color = colorPallete[submesh.ColorIndices[index]];
									if (color.A != 0)
										grid.Cells[acIndex]++;
								}
							}
						}
					}
				}
			}
		}
	}



	static std::vector<std::future<bool>> FillAccelerationGrid(VoxelMeshAccelerationGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh, uint32_t scale, uint32_t axisCells)
	{
		auto& pool = Application::Get().GetThreadPool();
		std::vector<std::future<bool>> futures;
		futures.reserve(axisCells * axisCells * axisCells);

		for (uint32_t x = 0; x < grid.Width; x += axisCells)
		{
			for (uint32_t y = 0; y < grid.Height; y += axisCells)
			{
				for (uint32_t z = 0; z < grid.Depth; z += axisCells)
				{
					const glm::uvec3 cellStart(x, y, z);
					const glm::uvec3 cellEnd(x + axisCells, y + axisCells, z + axisCells);
					futures.push_back(pool.SubmitJob([grid, colorPallete, submesh, scale, cellStart, cellEnd]() {

						return true;
					}));
				}
			}
		}

		return futures;
	}


	static void InsertSubmeshInAccelerationGrid(VoxelMeshAccelerationGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh)
	{


		const float scale = grid.Size / submesh.VoxelSize;
		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					const uint16_t colorIndex = static_cast<uint16_t>(submesh.ColorIndices[index]);
					const VoxelColor color = colorPallete[colorIndex];
					if (color.A != 0)
					{
						const glm::vec3 voxelPosition = {
							x * submesh.VoxelSize,
							y * submesh.VoxelSize,
							z * submesh.VoxelSize
						};
						const glm::ivec3 accelerationGridCellStart = {
							std::floor(voxelPosition.x / grid.Size),
							std::floor(voxelPosition.y / grid.Size),
							std::floor(voxelPosition.z / grid.Size)
						};
						const glm::ivec3 accelerationGridCellEnd = {
							std::ceil((voxelPosition.x + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.y + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.z + submesh.VoxelSize) / grid.Size)
						};

						for (uint32_t tx = accelerationGridCellStart.x; tx < accelerationGridCellEnd.x; tx++)
						{
							for (uint32_t ty = accelerationGridCellStart.y; ty < accelerationGridCellEnd.y; ty++)
							{
								for (uint32_t tz = accelerationGridCellStart.z; tz < accelerationGridCellEnd.z; tz++)
								{
									const uint32_t accelerationGridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
									if (accelerationGridIndex < grid.Cells.size())
									{
										grid.Cells[accelerationGridIndex]++;
									}
								}
							}
						}
					}
				}
			}
		}
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
		m_HasAccelerationGrid(false),
		m_GeneratingAccelerationGrid(false)
	{
		memset(&m_ColorPallete, 0, sizeof(VoxelColor) * m_ColorPallete.size());
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

	bool VoxelProceduralMesh::GenerateAccelerationGrid(uint32_t scale)
	{
		if (m_GeneratingAccelerationGrid)
			return false;

		m_AccelerationGrid = generateAccelerationGrid(m_Submeshes[0], m_ColorPallete, scale);
		m_HasAccelerationGrid = true;
		m_Dirty = true;
		return true;
	}

	bool VoxelProceduralMesh::GenerateAccelerationGridAsync(uint32_t scale)
	{
		if (m_GeneratingAccelerationGrid)
			return false;

		m_HasAccelerationGrid = false;
		m_GeneratingAccelerationGrid = true;
		Ref<VoxelProceduralMesh> instance = this;
		auto& pool = Application::Get().GetThreadPool();
		pool.PushJob([instance, &submesh = m_Submeshes[0], colorPallete = m_ColorPallete, scale]() mutable {
			instance->m_AccelerationGrid = generateAccelerationGrid(submesh, colorPallete, scale);
			instance->m_HasAccelerationGrid = true;
			instance->m_GeneratingAccelerationGrid = false;
			instance->m_Dirty = true;
		});
		
		return true;
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
	bool VoxelProceduralMesh::HasAccelerationGrid() const
	{
		return m_HasAccelerationGrid;
	}
	std::unordered_map<uint32_t, VoxelMesh::DirtyRange> VoxelProceduralMesh::DirtySubmeshes() const
	{
		return std::move(m_DirtySubmeshes);
	}
	VoxelMeshAccelerationGrid VoxelProceduralMesh::generateAccelerationGrid(const VoxelSubmesh& submesh, const std::array<VoxelColor, 256>& colorPallete, uint32_t scale)
	{
		VoxelMeshAccelerationGrid accelerationGrid;
		accelerationGrid.Width = Math::RoundUp(submesh.Width, scale) / scale;
		accelerationGrid.Height = Math::RoundUp(submesh.Height, scale) / scale;
		accelerationGrid.Depth = Math::RoundUp(submesh.Depth, scale) / scale;
		accelerationGrid.Size = submesh.VoxelSize * scale;
		accelerationGrid.Cells.resize(accelerationGrid.Width * accelerationGrid.Height * accelerationGrid.Depth, 0);


		for (uint32_t ax = 0; ax < accelerationGrid.Width; ++ax)
		{
			for (uint32_t ay = 0; ay < accelerationGrid.Height; ++ay)
			{
				for (uint32_t az = 0; az < accelerationGrid.Depth; ++az)
				{
					const uint32_t acIndex = Index3D(ax, ay, az, accelerationGrid.Width, accelerationGrid.Height);
					for (uint32_t x = 0; x < scale; ++x)
					{
						for (uint32_t y = 0; y < scale; ++y)
						{
							for (uint32_t z = 0; z < scale; ++z)
							{
								const uint32_t index = Index3D(ax * scale + x, ay * scale + y, az * scale + z, submesh.Width, submesh.Height);
								if (index < submesh.ColorIndices.size())
								{
									accelerationGrid.Cells[acIndex]++;
								}
							}
						}
					}
				}
			}
		}


		return accelerationGrid;
	}
}