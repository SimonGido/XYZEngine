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
						const glm::ivec3 AccelerationGridCellStart = {
							std::floor(voxelPosition.x / grid.Size),
							std::floor(voxelPosition.y / grid.Size),
							std::floor(voxelPosition.z / grid.Size)
						};
						const glm::ivec3 AccelerationGridCellEnd = {
							std::ceil((voxelPosition.x + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.y + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.z + submesh.VoxelSize) / grid.Size)
						};

						for (uint32_t tx = AccelerationGridCellStart.x; tx < AccelerationGridCellEnd.x; tx++)
						{
							for (uint32_t ty = AccelerationGridCellStart.y; ty < AccelerationGridCellEnd.y; ty++)
							{
								for (uint32_t tz = AccelerationGridCellStart.z; tz < AccelerationGridCellEnd.z; tz++)
								{
									const uint32_t AccelerationGridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
									if (AccelerationGridIndex < grid.Cells.size())
									{
										grid.Cells[AccelerationGridIndex]++;
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

	bool VoxelProceduralMesh::GenerateAccelerationGridAsync(float size)
	{
		if (m_GeneratingAccelerationGrid)
			return false;

		m_HasAccelerationGrid = false;
		m_GeneratingAccelerationGrid = true;
		Ref<VoxelProceduralMesh> instance = this;
		auto& pool = Application::Get().GetThreadPool();
		pool.PushJob([instance, submeshes = m_Submeshes, colorPallete = m_ColorPallete, size]() mutable {
			instance->m_AccelerationGrid = generateAccelerationGrid(submeshes, colorPallete, size);
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
	VoxelMeshAccelerationGrid VoxelProceduralMesh::generateAccelerationGrid(const std::vector<VoxelSubmesh>& submeshes, const std::array<VoxelColor, 256>& colorPallete, float size)
	{
		AABB aabb(glm::vec3(0), glm::vec3(FLT_MIN));
		for (auto& submesh : submeshes)
		{
			aabb.Max.x = std::max(static_cast<float>(submesh.Width) * submesh.VoxelSize, aabb.Max.x);
			aabb.Max.y = std::max(static_cast<float>(submesh.Height) * submesh.VoxelSize, aabb.Max.y);
			aabb.Max.z = std::max(static_cast<float>(submesh.Depth) * submesh.VoxelSize, aabb.Max.z);
		}
		VoxelMeshAccelerationGrid AccelerationGrid = CreateAccelerationGridFromAABB(aabb, size);
		for (auto& submesh : submeshes)
		{
			InsertSubmeshInAccelerationGrid(AccelerationGrid, colorPallete, submesh);
		}
		return AccelerationGrid;
	}
}