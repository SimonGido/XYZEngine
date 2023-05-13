#include "stdafx.h"
#include "Octree.h"


namespace XYZ {
	Octree::Octree(const AABB& aabb, uint32_t maxDepth)
		:
		m_MaxDepth(maxDepth)
	{
		auto& root = m_Nodes.emplace_back();
		root.BoundingBox = aabb;
	}
	void Octree::InsertData(const AABB& aabb, int32_t data)
	{		
		constexpr float resizeConstant = 1.5f;
		OctreeNode* root = &m_Nodes[0];
		if (!root->BoundingBox.Contains(aabb))
		{
			AABB newAABB;
			newAABB.Min.x = std::min(root->BoundingBox.Min.x, aabb.Min.x) * resizeConstant;
			newAABB.Min.y = std::min(root->BoundingBox.Min.y, aabb.Min.y) * resizeConstant;
			newAABB.Min.z = std::min(root->BoundingBox.Min.z, aabb.Min.z) * resizeConstant;

			newAABB.Max.x = std::max(root->BoundingBox.Max.x, aabb.Max.x) * resizeConstant;
			newAABB.Max.y = std::max(root->BoundingBox.Max.y, aabb.Max.y) * resizeConstant;
			newAABB.Max.z = std::max(root->BoundingBox.Max.z, aabb.Max.z) * resizeConstant;

			Octree newOctree = resize(newAABB, *this);
			m_Nodes = std::move(newOctree.m_Nodes);
		}

		std::stack<int32_t> nodesToIterate;
		nodesToIterate.push(0); // Start from root

		while (!nodesToIterate.empty())
		{
			int32_t nodeIndex = nodesToIterate.top();
			nodesToIterate.pop();

			OctreeNode* node = &m_Nodes[nodeIndex];
		
			bool canInsert = true;
			if (!node->IsLeaf && node->Depth != m_MaxDepth)
			{
				splitNode(nodeIndex);
			}

			node = &m_Nodes[nodeIndex];
			if (node->IsLeaf)
			{
				for (int32_t childIndex : node->Children)
				{
					OctreeNode* childNode = &m_Nodes[childIndex];
					if (childNode->BoundingBox.Contains(aabb))
					{
						nodesToIterate.push(childIndex);
						canInsert = false;
						break;
					}
				}
			}
			if (canInsert)
			{
				node->Data.push_back({ data, aabb });
				break;
			}
		}
	}

	void Octree::Clear()
	{
		for (auto& node : m_Nodes)
		{
			node.Data.clear();
		}
	}

	int32_t Octree::FindNode(const glm::vec3& position)
	{
		std::stack<int32_t> nodesToIterate;
		nodesToIterate.push(0); // Start from root

		while (!nodesToIterate.empty())
		{
			int32_t nodeIndex = nodesToIterate.top();
			nodesToIterate.pop();

			OctreeNode* node = &m_Nodes[nodeIndex];

			bool canReturn = true;
			if (node->IsLeaf)
			{
				for (int32_t childIndex : node->Children)
				{
					OctreeNode* childNode = &m_Nodes[childIndex];

					if (Math::PointInBox(position, childNode->BoundingBox.Min, childNode->BoundingBox.Max))
					{
						nodesToIterate.push(childIndex);
						canReturn = false;
						break;
					}
				}
			}
			if (canReturn)
			{
				if (Math::PointInBox(position, node->BoundingBox.Min, node->BoundingBox.Max))
					return nodeIndex;
			}
		}
		return 0;
	}

	

	void Octree::splitNode(int32_t nodeIndex)
	{
		OctreeNode* parent = &m_Nodes[nodeIndex];
		parent->IsLeaf = true;
		const glm::vec3 halfSize = (parent->BoundingBox.Max - parent->BoundingBox.Min) / 2.0f;

		uint32_t childIndex = 0;
		for (uint32_t z = 0; z < 2; ++z)
		{
			for (uint32_t y = 0; y < 2; ++y)
			{
				for (uint32_t x = 0; x < 2; ++x)
				{
					parent->Children[childIndex] = static_cast<int32_t>(m_Nodes.size());
					auto& child = m_Nodes.emplace_back();
					parent = &m_Nodes[nodeIndex];
					
					child.BoundingBox.Min = parent->BoundingBox.Min + glm::vec3(x, y, z) * halfSize;
					child.BoundingBox.Max = child.BoundingBox.Min + halfSize;
					child.Depth = parent->Depth + 1;
					childIndex++;
				}
			}
		}
	}
	Octree Octree::resize(const AABB& aabb, const Octree& originalOctree)
	{
		Octree newOctree(aabb, originalOctree.m_MaxDepth);
		for (auto& node : originalOctree.m_Nodes)
		{
			for (auto& data : node.Data)
				newOctree.InsertData(data.BoundingBox, data.Data);
		}
		return newOctree;
	}
}