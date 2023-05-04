#include "stdafx.h"
#include "VoxelMesh.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}

	static VoxelMeshTopGrid GenerateTopGrid(const VoxelSubmesh& submesh, const std::array<VoxelColor, 256>& colorPallete, float size)
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
		m_Dirty(false)
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
			// TODO: single top grid for all submeshes
			m_TopGrid = GenerateTopGrid(submesh, m_ColorPallete, 64.0f);
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
	std::unordered_map<uint32_t, VoxelMesh::DirtyRange> VoxelProceduralMesh::DirtySubmeshes() const
	{
		return std::move(m_DirtySubmeshes);
	}
}