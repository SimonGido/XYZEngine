#include "stdafx.h"
#include "VoxelMesh.h"

#include "XYZ/Utils/Math/AABB.h"


namespace XYZ {
	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}
	
	struct VoxelOctreeNode
	{
		static constexpr uint32_t MultiColor = 256;

		uint32_t	Width;
		uint32_t	Height;
		uint32_t	Depth;
		uint32_t	X, Y, Z;

		VoxelOctreeNode* Children[8];

		bool		IsLeaf = false;

		uint32_t	ColorIndex;
		uint32_t	DepthFromRoot = 0;

		std::vector<glm::ivec3> Voxels;
	};

	class VoxelOctree
	{
	public:
		VoxelOctree(uint32_t width, uint32_t height, uint32_t depth, uint32_t maxDepth = 8)
			:
			m_MaxDepth(maxDepth)
		{
			m_Root.Width = width;
			m_Root.Height = height;
			m_Root.Depth = depth;
			m_Root.X = 0;
			m_Root.Y = 0;
			m_Root.Z = 0;
		}
		~VoxelOctree()
		{
			for (auto node : m_AllNodes)
				delete node;
		}

		void InsertVoxel(const glm::ivec3& voxel, uint32_t colorIndex)
		{
			VoxelOctreeNode* node = FindNode(voxel);
			XYZ_ASSERT(node, "Failed to find node");
			insertVoxel(node, voxel, colorIndex);
		}

		VoxelOctreeNode* FindNode(const glm::ivec3& voxel)
		{
			return findNode(&m_Root, voxel);
		}

		VoxelOctreeNode* GetRoot() { return &m_Root; }

		const std::vector<VoxelOctreeNode*>& GetAllNodes() const { return m_AllNodes; }
	private:
		void insertVoxel(VoxelOctreeNode* node, const glm::ivec3& voxel, uint32_t colorIndex)
		{
			if (node->Voxels.empty()) // Node is empty
			{
				node->ColorIndex = colorIndex;
			}
			if (node->ColorIndex != colorIndex && node->ColorIndex != VoxelOctreeNode::MultiColor) // Check if color matches or if node is not multicolor
			{
				if (node->Depth != m_MaxDepth)
				{
					splitNode(node);
					node = findNode(node, voxel);
					insertVoxel(node, voxel, colorIndex);
				}
				else // Max depth was reached, node becomes multi color
				{
					node->Voxels.push_back(voxel);
					node->ColorIndex = VoxelOctreeNode::MultiColor;
				}
			}
			else
			{
				node->Voxels.push_back(voxel);
			}
		}
		void splitNode(VoxelOctreeNode* node)
		{
			node->IsLeaf = true;
			uint32_t width = node->Width / 2;
			uint32_t height = node->Height / 2;
			uint32_t depth = node->Depth / 2;

			uint32_t childIndex = 0;
			for (uint32_t z = 0; z < 2; ++z)
			{
				for (uint32_t y = 0; y < 2; ++y)
				{
					for (uint32_t x = 0; x < 2; ++x)
					{
						node->Children[childIndex] = new VoxelOctreeNode();
						node->Children[childIndex]->Width = width;
						node->Children[childIndex]->Height = height;
						node->Children[childIndex]->Depth = depth;

						node->Children[childIndex]->X = node->X + x * width;
						node->Children[childIndex]->Y = node->Y + y * height;
						node->Children[childIndex]->Z = node->Z + z * depth;
						node->Children[childIndex]->DepthFromRoot = node->DepthFromRoot + 1;
						m_AllNodes.push_back(node->Children[childIndex]);
						childIndex++;
					}
				}
			}
			for (auto voxel : node->Voxels) // Reinsert existing voxels in new created nodes
			{			
				for (auto child : node->Children)
				{
					if (voxel.x >= child->X
						&& voxel.y >= child->Y
						&& voxel.z >= child->Z
						&& voxel.x < child->X + child->Width
						&& voxel.y < child->Y + child->Height
						&& voxel.z < child->Z + child->Depth
						)
					{
						insertVoxel(child, voxel, node->ColorIndex);
						break;
					}
				}
			}
			node->Voxels.clear();
		}

		VoxelOctreeNode* findNode(VoxelOctreeNode* node, const glm::ivec3& voxel)
		{
			std::stack<VoxelOctreeNode*> nodesToIterate;
			nodesToIterate.push(node);
			while (!nodesToIterate.empty())
			{
				VoxelOctreeNode* tempNode = nodesToIterate.top();
				nodesToIterate.pop();

				if (tempNode->IsLeaf)
				{
					for (auto child : tempNode->Children)
					{
						if (voxel.x		>= child->X
							&& voxel.y	>= child->Y
							&& voxel.z	>= child->Z
							&& voxel.x	<  child->X + child->Width
							&& voxel.y	<  child->Y + child->Height
							&& voxel.z	<  child->Z + child->Depth
							)
						{
							nodesToIterate.push(child);
							break;
						}
					}
				}
				else
				{
					if (voxel.x		>= tempNode->X
						&& voxel.y	>= tempNode->Y
						&& voxel.z	>= tempNode->Z
						&& voxel.x	<  tempNode->X + tempNode->Width
						&& voxel.y	<  tempNode->Y + tempNode->Height
						&& voxel.z	<  tempNode->Z + tempNode->Depth
						)
					{
						return tempNode;
					}
				}
			}

			return nullptr;
		}

	private:
		VoxelOctreeNode m_Root;
		uint32_t   m_MaxDepth;

		std::vector<VoxelOctreeNode*> m_AllNodes;
	};

	static std::vector<VoxelSubmesh> CompressVoxelSubmesh(const VoxelSubmesh& submesh, uint32_t scale, std::vector<glm::mat4>& transforms)
	{
		std::vector<VoxelSubmesh> result;
		
		VoxelOctree octree(submesh.Width, submesh.Height, submesh.Depth, scale);
		for (uint32_t x = 0; x < submesh.Width; x++)
		{
			for (uint32_t y = 0; y < submesh.Height; y++)
			{
				for (uint32_t z = 0; z < submesh.Depth; z++)
				{
					uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					uint32_t colorIndex = static_cast<uint32_t>(submesh.ColorIndices[index]);
					octree.InsertVoxel(glm::vec3(x, y, z), colorIndex);
				}
			}
		}
		uint32_t voxelCount = 0;
		for (auto node : octree.GetAllNodes())
		{
			if (!node->IsLeaf)
			{
				int32_t halfX = node->X / 2;
				int32_t halfY = node->Y / 2;
				int32_t halfZ = node->Z / 2;

				
				if (node->ColorIndex != VoxelOctreeNode::MultiColor && node->ColorIndex != 0)
				{
					
					VoxelSubmesh& childSubmesh = result.emplace_back();
					const uint32_t minDimension = std::min(node->Width, std::min(node->Height, node->Depth));
					childSubmesh.Width = node->Width / minDimension;
					childSubmesh.Height = node->Height / minDimension;
					childSubmesh.Depth = node->Depth / minDimension;
					childSubmesh.VoxelSize = submesh.VoxelSize * minDimension;
					childSubmesh.ColorIndices.resize(childSubmesh.Width * childSubmesh.Height * childSubmesh.Depth, static_cast<uint8_t>(node->ColorIndex));			
					voxelCount += 1;
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(node->Z, node->Y, node->X) * submesh.VoxelSize);
					transforms.push_back(transform);
				}
				else
				{
					VoxelSubmesh& childSubmesh = result.emplace_back();
					childSubmesh.Width = node->Width;
					childSubmesh.Height = node->Height;
					childSubmesh.Depth = node->Depth;
					childSubmesh.VoxelSize = submesh.VoxelSize;

					for (auto voxel : node->Voxels)
					{
						uint32_t index = Index3D(voxel.x, voxel.y, voxel.z, submesh.Width, submesh.Height);
						childSubmesh.ColorIndices.push_back(submesh.ColorIndices[index]);
					}
					voxelCount += node->Voxels.size();
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(node->Z, node->Y, node->X) * submesh.VoxelSize);
					transforms.push_back(transform);
				}
			}
		}
		int32_t savedSpace = submesh.ColorIndices.size() - voxelCount;
		return result;
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
	void VoxelProceduralMesh::Compress(uint32_t scale)
	{
		std::vector<glm::mat4> transforms;
		m_Submeshes = CompressVoxelSubmesh(m_Submeshes[0], scale, transforms);
		m_Instances.clear();
		uint32_t submeshIndex = 0;
		for (auto& transform : transforms)
		{
			auto& instance = m_Instances.emplace_back();
			instance.Transform = transform;
			instance.SubmeshIndex = submeshIndex++;
		}
		m_Dirty = true;
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