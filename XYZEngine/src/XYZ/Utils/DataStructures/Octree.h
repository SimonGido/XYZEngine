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

		bool IsLeaf = false;
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

}