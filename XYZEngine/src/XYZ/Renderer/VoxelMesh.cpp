#include "stdafx.h"
#include "VoxelMesh.h"

#include "XYZ/Utils/Math/AABB.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}

	static VoxelMeshTopGrid GenerateTopGridFromAABB(const VoxelSubmesh& submesh, const std::array<VoxelColor, 256>& colorPallete, float size)
	{
		VoxelMeshTopGrid topGrid;

		const float scale = size / submesh.VoxelSize;
		const uint32_t width = std::ceil((float)submesh.Width / scale);
		const uint32_t height = std::ceil((float)submesh.Height / scale);
		const uint32_t depth = std::ceil((float)submesh.Depth / scale);

		topGrid.MaxTraverses = std::sqrtl(std::powl(width, 2) + std::powl(height, 2) + std::powl(depth, 2)) * 2;
		topGrid.Width = width;
		topGrid.Height = height;
		topGrid.Depth = depth;
		topGrid.Size = size;
		topGrid.VoxelCount.resize(width * height * depth);
		memset(topGrid.VoxelCount.data(), 0, topGrid.VoxelCount.size() * sizeof(uint32_t));

		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					const VoxelColor color = colorPallete[submesh.ColorIndices[index]];
					if (color.A != 0)
					{
						const glm::vec3 voxelPosition = {
							x * submesh.VoxelSize,
							y * submesh.VoxelSize,
							z * submesh.VoxelSize
						};
						const glm::ivec3 topGridCellStart = {
							std::floor(voxelPosition.x / size),
							std::floor(voxelPosition.y / size),
							std::floor(voxelPosition.z / size)
						};
						const glm::ivec3 topGridCellEnd = {
							std::ceil((voxelPosition.x + submesh.VoxelSize) / size),
							std::ceil((voxelPosition.y + submesh.VoxelSize) / size),
							std::ceil((voxelPosition.z + submesh.VoxelSize) / size)
						};
						
						for (uint32_t tx = topGridCellStart.x; tx < topGridCellEnd.x; tx++)
						{
							for (uint32_t ty = topGridCellStart.y; ty < topGridCellEnd.y; ty++)
							{
								for (uint32_t tz = topGridCellStart.z; tz < topGridCellEnd.z; tz++)
								{
									const uint32_t topgridIndex = Index3D(tx, ty, tz, width, height);
									topGrid.VoxelCount[topgridIndex]++;
								}
							}
						}
					}
				}
			}
		}
		uint32_t emptyCount = 0;
		for (auto count : topGrid.VoxelCount)
		{
			if (count == 0)
				emptyCount++;
		}

		return topGrid;
	}


	static VoxelMeshTopGrid CreateTopGridFromAABB(const AABB& aabb, float size)
	{
		VoxelMeshTopGrid topGrid;

		const uint32_t width = static_cast<uint32_t>(std::ceil(aabb.Max.x - aabb.Min.x) / size);
		const uint32_t height = static_cast<uint32_t>(std::ceil(aabb.Max.y - aabb.Min.y) / size);
		const uint32_t depth = static_cast<uint32_t>(std::ceil(aabb.Max.z - aabb.Min.z) / size);

		topGrid.MaxTraverses = std::sqrtl(std::powl(width, 2) + std::powl(height, 2) + std::powl(depth, 2)) * 2;
		topGrid.Width = width;
		topGrid.Height = height;
		topGrid.Depth = depth;
		topGrid.Size = size;
		topGrid.VoxelCount.resize(width * height * depth);
		memset(topGrid.VoxelCount.data(), 0, topGrid.VoxelCount.size() * sizeof(uint32_t));
		
		return topGrid;
	}

	static void InsertSubmeshInTopGrid(VoxelMeshTopGrid& grid, const std::array<VoxelColor, 256>& colorPallete, const VoxelSubmesh& submesh)
	{
		const float scale = grid.Size / submesh.VoxelSize;
		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					const VoxelColor color = colorPallete[submesh.ColorIndices[index]];
					if (color.A != 0)
					{
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
						const glm::ivec3 topGridCellEnd = {
							std::ceil((voxelPosition.x + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.y + submesh.VoxelSize) / grid.Size),
							std::ceil((voxelPosition.z + submesh.VoxelSize) / grid.Size)
						};

						for (uint32_t tx = topGridCellStart.x; tx < topGridCellEnd.x; tx++)
						{
							for (uint32_t ty = topGridCellStart.y; ty < topGridCellEnd.y; ty++)
							{
								for (uint32_t tz = topGridCellStart.z; tz < topGridCellEnd.z; tz++)
								{
									const uint32_t topgridIndex = Index3D(tx, ty, tz, grid.Width, grid.Height);
									grid.VoxelCount[topgridIndex]++;
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
		GenerateTopGridAsync(64.0f);
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