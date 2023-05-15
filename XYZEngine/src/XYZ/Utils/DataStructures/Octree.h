#pragma once
#include "XYZ/Utils/Math/AABB.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct OctreeData
	{
		int32_t Data;
		AABB    BoundingBox;
	};

	struct OctreeNode
	{
		AABB	 BoundingBox;
		uint32_t Depth = 0;
		int32_t  Children[8]{ -1 };

		bool IsLeaf = true;
		std::vector<OctreeData> Data;
	};


	class Octree
	{
	public:
		Octree(const AABB& aabb, uint32_t maxDepth);

		void InsertData(const AABB& aabb, int32_t data);
		void Clear();

		int32_t FindNode(const glm::vec3& position);

		const std::vector<OctreeNode>& GetNodes() const { return m_Nodes; }
	private:

		void splitNode(int32_t nodeIndex);
		static Octree resize(const AABB& aabb, const Octree& originalOctree);

	private:
		std::vector<OctreeNode> m_Nodes;
		uint32_t m_MaxDepth;
	};

	struct VoxelOctreeNode
	{
		uint32_t	Size;
		uint32_t	X, Y, Z;

		uint32_t	Children[8];
		uint32_t	ColorIndex = 0;
		uint32_t	IsLeaf = true;
		Padding<8> Padding;
	};

	class VoxelOctree
	{
	public:
		VoxelOctree(uint32_t width, uint32_t height, uint32_t depth);


		void InsertVoxels(const std::vector<uint8_t>& voxels, uint32_t width, uint32_t height);

		const std::vector<VoxelOctreeNode>& GetNodes() const { return m_Nodes; }
	private:

		void splitNode(int32_t nodeIndex);

	private:
		std::vector<VoxelOctreeNode> m_Nodes;
	};
}