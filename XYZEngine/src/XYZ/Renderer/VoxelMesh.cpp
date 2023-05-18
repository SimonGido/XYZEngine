#include "stdafx.h"
#include "VoxelMesh.h"

#include "XYZ/Utils/Math/AABB.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}


	static void CompressSubmesh(VoxelSubmesh& compressed, const VoxelSubmesh& submesh, uint32_t scale)
	{
		compressed.CompressedCells.resize(compressed.Width * compressed.Height * compressed.Depth);
		compressed.CompressedVoxelCount = 0;
		compressed.CompressScale = scale;
		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					const uint32_t cellX = x / scale;
					const uint32_t cellY = y / scale;
					const uint32_t cellZ = z / scale;
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					const uint8_t voxel = submesh.ColorIndices[index];
					const uint32_t cellIndex = Index3D(cellX, cellY, cellZ, compressed.Width, compressed.Height);
					if (cellIndex < compressed.CompressedCells.size())
					{
						if (compressed.CompressedCells[cellIndex].Voxels.size() == 1 
						 && compressed.CompressedCells[cellIndex].Voxels[0] == voxel)
							continue;

						compressed.CompressedCells[cellIndex].Voxels.push_back(voxel);
					}
				}
			}
		}
		uint32_t voxelOffset = 0;
		for (auto& cell : compressed.CompressedCells)
		{
			voxelOffset += cell.Voxels.size();
		}
		compressed.CompressedVoxelCount = voxelOffset;
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


	bool VoxelProceduralMesh::Compress(uint32_t submeshIndex, uint32_t scale)
	{
		auto& submesh = m_Submeshes[submeshIndex];
		VoxelSubmesh compressed;
		compressed.Width = Math::RoundUp(submesh.Width, scale) / scale;
		compressed.Height = Math::RoundUp(submesh.Height, scale) / scale;
		compressed.Depth = Math::RoundUp(submesh.Depth, scale) / scale;
		compressed.VoxelSize = scale * submesh.VoxelSize;
		CompressSubmesh(compressed, submesh, scale);
		m_Submeshes[submeshIndex] = std::move(compressed);
		m_Dirty = true;
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

	std::unordered_map<uint32_t, VoxelMesh::DirtyRange> VoxelProceduralMesh::DirtySubmeshes() const
	{
		return std::move(m_DirtySubmeshes);
	}
	
}