#include "stdafx.h"
#include "DynamicTree.h"

#include "XYZ/Renderer/Renderer2D.h"



// Copied Box2D implementation of the dynamic tree
// https://github.com/behdad/box2d/blob/master/Box2D/Box2D/Collision/b2DynamicTree.cpp


namespace XYZ {
	bool DynamicTree::RayCast(const Ray& ray, uint32_t& result)
	{
		std::stack<int32_t> stack;
		stack.push(m_RootIndex);
		while (!stack.empty())
		{
			int32_t index = stack.top();
			stack.pop();
			if (!ray.IntersectsAABB(m_Nodes[index].Box))
			{
				continue;
			}
			if (m_Nodes[index].IsLeaf())
			{
				uint32_t objectIndex = m_Nodes[index].ObjectIndex;
				if (ray.IntersectsAABB(m_Nodes[index].Box))
				{
					result = objectIndex;
					return true;
				}
			}
			else
			{
				stack.push(m_Nodes[index].FirstChild);
				stack.push(m_Nodes[index].SecondChild);
			}
		}
		return false;
	}
	void DynamicTree::Query(const CollisionCallback& callback,const AABB& aabb)
	{
		std::stack<int32_t> stack;
		stack.push(m_RootIndex);
		while (!stack.empty())
		{
			int32_t index = stack.top();
			stack.pop();
			if (index == NULL_NODE)
				continue;

			const Node& node = m_Nodes[index];
			if (node.Box.Intersect(aabb))
			{
				if (node.IsLeaf())
				{
					bool proceed = callback(index);
					if (proceed)
						return;
				}
				else
				{
					stack.push(node.FirstChild);
					stack.push(node.SecondChild);
				}
			}
		}
	}
	int32_t DynamicTree::Insert(uint32_t objectIndex, const AABB& box)
	{
		int32_t leaf = m_Nodes.Insert({ box , objectIndex });
		
		if (m_MovedNodes.size() < m_Nodes.Range())
			m_MovedNodes.resize(m_Nodes.Range());
		insertLeaf(leaf);
		return leaf;
	}
	void DynamicTree::Move(int32_t index, const glm::vec2& displacement)
	{
		removeLeaf(index);

		m_Nodes[index].Box.Min.x += displacement.x;
		m_Nodes[index].Box.Min.y += displacement.y;
		m_Nodes[index].Box.Max.x += displacement.x;
		m_Nodes[index].Box.Max.y += displacement.y;

		m_MovedNodes[index] = true;
		insertLeaf(index);
	}
	void DynamicTree::Remove(int32_t index)
	{		
		removeLeaf(index);
		m_Nodes.Erase(index);
	}

	void DynamicTree::SubmitToRenderer()
	{
		std::stack<int32_t> stack;
		if (m_RootIndex != NULL_NODE)
			stack.push(m_RootIndex);
		while (!stack.empty())
		{
			int32_t index = stack.top();
			stack.pop();

			AABB box = m_Nodes[index].Box;

			Renderer2D::SubmitLine(box.Min, glm::vec3(box.Max.x, box.Min.y, box.Min.z));
			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Min.z));
			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Min.z));
			Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), box.Min);


			Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z));
			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z));
			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z));
			Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Min.y, box.Max.z));


			Renderer2D::SubmitLine(box.Min, glm::vec3(box.Min.x, box.Min.y, box.Max.z));
			Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z));


			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z));
			Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z));


			if (m_Nodes[index].IsLeaf())
			{
				
			}
			else
			{
				stack.push(m_Nodes[index].FirstChild);
				stack.push(m_Nodes[index].SecondChild);
			}
		}
	}

	void DynamicTree::CleanMovedNodes()
	{
		for (auto& node : m_MovedNodes)
			node = false;
	}

	void DynamicTree::insertLeaf(int32_t leaf)
	{
		if (m_RootIndex == NULL_NODE)
		{
			m_RootIndex = leaf;
			return;
		}

		AABB leafAABB = m_Nodes[leaf].Box;
		int32_t index = m_RootIndex;
		while (!m_Nodes[index].IsLeaf())
		{
			int32_t firstChild = m_Nodes[index].FirstChild;
			int32_t secondChild = m_Nodes[index].SecondChild;

			float area = m_Nodes[index].Box.GetPerimeter();

			AABB combinedAABB = AABB::Union(m_Nodes[index].Box, leafAABB);
			float combinedArea = combinedAABB.GetPerimeter();

			// Cost of creating a new parent for this node and the new leaf
			float cost = 2.0f * combinedArea;

			// Minimum cost of pushing the leaf further down the tree
			float inheritanceCost = 2.0f * (combinedArea - area);

			// Cost of descending into child1
			float cost1;
			if (m_Nodes[firstChild].IsLeaf())
			{
				AABB aabb = AABB::Union(leafAABB, m_Nodes[firstChild].Box);
				cost1 = aabb.GetPerimeter() + inheritanceCost;
			}
			else
			{
				AABB aabb = AABB::Union(leafAABB, m_Nodes[firstChild].Box);
				float oldArea = m_Nodes[firstChild].Box.GetPerimeter();
				float newArea = aabb.GetPerimeter();
				cost1 = (newArea - oldArea) + inheritanceCost;
			}

			// Cost of descending into child2
			float cost2;
			if (m_Nodes[secondChild].IsLeaf())
			{
				AABB aabb = AABB::Union(leafAABB, m_Nodes[secondChild].Box);
				cost2 = aabb.GetPerimeter() + inheritanceCost;
			}
			else
			{
				AABB aabb = AABB::Union(leafAABB, m_Nodes[secondChild].Box);
				float oldArea = m_Nodes[secondChild].Box.GetPerimeter();
				float newArea = aabb.GetPerimeter();
				cost2 = newArea - oldArea + inheritanceCost;
			}

			// Descend according to the minimum cost.
			if (cost < cost1 && cost < cost2)
			{
				break;
			}

			// Descend
			if (cost1 < cost2)
			{
				index = firstChild;
			}
			else
			{
				index = secondChild;
			}
		}
		int32_t sibling = index;

		// Create a new parent.
		int32_t oldParent = m_Nodes[sibling].ParentIndex;
		int32_t newParent = m_Nodes.Insert({ AABB::Union(leafAABB, m_Nodes[sibling].Box), 0, oldParent });
		m_Nodes[newParent].Height = m_Nodes[sibling].Height + 1;

		if (oldParent != NULL_NODE)
		{
			// The sibling was not the root.
			if (m_Nodes[oldParent].FirstChild == sibling)
			{
				m_Nodes[oldParent].FirstChild = newParent;
			}
			else
			{
				m_Nodes[oldParent].SecondChild = newParent;
			}

			m_Nodes[newParent].FirstChild = sibling;
			m_Nodes[newParent].SecondChild = leaf;
			m_Nodes[sibling].ParentIndex = newParent;
			m_Nodes[leaf].ParentIndex = newParent;
		}
		else
		{
			// The sibling was the root.
			m_Nodes[newParent].FirstChild = sibling;
			m_Nodes[newParent].SecondChild = leaf;
			m_Nodes[sibling].ParentIndex = newParent;
			m_Nodes[leaf].ParentIndex = newParent;
			m_RootIndex = newParent;
		}

		// Walk back up the tree fixing heights and AABBs
		index = m_Nodes[leaf].ParentIndex;
		while (index != NULL_NODE)
		{
			index = balance(index);

			int32_t child1 = m_Nodes[index].FirstChild;
			int32_t child2 = m_Nodes[index].SecondChild;

			m_Nodes[index].Height = 1 + std::max(m_Nodes[child1].Height, m_Nodes[child2].Height);
			m_Nodes[index].Box = AABB::Union(m_Nodes[child1].Box, m_Nodes[child2].Box);

			index = m_Nodes[index].ParentIndex;
		}
	}

	void DynamicTree::removeLeaf(int32_t index)
	{
		if (index == m_RootIndex)
		{
			m_RootIndex = NULL_NODE;
			return;
		}
		int32_t parent = m_Nodes[index].ParentIndex;
		int32_t grandParent = m_Nodes[parent].ParentIndex;
		int32_t sibling;
		if (m_Nodes[parent].FirstChild == index)
		{
			sibling = m_Nodes[parent].SecondChild;
		}
		else
		{
			sibling = m_Nodes[parent].FirstChild;
		}
		if (grandParent != NULL_NODE)
		{
			if (m_Nodes[grandParent].FirstChild == parent)
				m_Nodes[grandParent].FirstChild = sibling;
			else
				m_Nodes[grandParent].SecondChild = sibling;

			m_Nodes[sibling].ParentIndex = grandParent;
			m_Nodes.Erase(parent);

			int32_t tmpIndex = grandParent;
			while (tmpIndex != NULL_NODE)
			{
				tmpIndex = balance(tmpIndex);
				int32_t firstChild = m_Nodes[tmpIndex].FirstChild;
				int32_t secondChild = m_Nodes[tmpIndex].SecondChild;
			
				m_Nodes[tmpIndex].Box = AABB::Union(m_Nodes[firstChild].Box, m_Nodes[secondChild].Box);
				m_Nodes[tmpIndex].Height = 1 + std::max(m_Nodes[firstChild].Height, m_Nodes[secondChild].Height);
						
				tmpIndex = m_Nodes[tmpIndex].ParentIndex;
			}
		}
		else
		{
			m_RootIndex = sibling;
			m_Nodes[sibling].ParentIndex = NULL_NODE;
			m_Nodes.Erase(parent);
		}
	}

	int32_t DynamicTree::balance(int32_t iA)
	{
		Node* A = &m_Nodes[iA];
		if (A->IsLeaf() || A->Height < 2)
		{
			return iA;
		}
		int32_t iB = A->FirstChild;
		int32_t iC = A->SecondChild;

		Node* B = &m_Nodes[iB];
		Node* C = &m_Nodes[iC];

		int32_t bal = C->Height - B->Height;
		// Rotate C up
		if (bal > 1)
		{
			int32_t iF = C->FirstChild;
			int32_t iG = C->SecondChild;
			Node* F = &m_Nodes[iF];
			Node* G = &m_Nodes[iG];

			C->FirstChild = iA;
			C->ParentIndex = A->ParentIndex;
			A->ParentIndex = iC;
			if (C->ParentIndex != NULL_NODE)
			{
				if (m_Nodes[C->ParentIndex].FirstChild == iA)
					m_Nodes[C->ParentIndex].FirstChild = iC;
				else
					m_Nodes[C->ParentIndex].SecondChild = iC;
			}
			else
				m_RootIndex = iC;

			if (F->Height > G->Height)
			{
				C->SecondChild = iF;
				A->SecondChild = iG;
				G->ParentIndex = iA;
				A->Box = AABB::Union(B->Box, G->Box);
				C->Box = AABB::Union(A->Box, F->Box);

				A->Height = 1 + std::max(B->Height, G->Height);
				C->Height = 1 + std::max(A->Height, F->Height);
			}
			else
			{
				C->SecondChild = iG;
				A->SecondChild = iF;
				F->ParentIndex = iA;
				A->Box = AABB::Union(B->Box, F->Box);
				C->Box = AABB::Union(A->Box, G->Box);

				A->Height = 1 + std::max(B->Height, F->Height);
				C->Height = 1 + std::max(A->Height, G->Height);
			}
			return iC;
		}
		
		if (bal < -1)
		{
			int32_t iD = B->FirstChild;
			int32_t iE = B->SecondChild;
			Node* D = &m_Nodes[iD];
			Node* E = &m_Nodes[iE];
		
			// Swap A and B
			B->FirstChild = iA;
			B->ParentIndex = A->ParentIndex;
			A->ParentIndex = iB;

			// A's old parent should point to B
			if (B->ParentIndex != NULL_NODE)
			{
				if (m_Nodes[B->ParentIndex].FirstChild == iA)
				{
					m_Nodes[B->ParentIndex].FirstChild = iB;
				}
				else
				{
					m_Nodes[B->ParentIndex].SecondChild = iB;
				}
			}
			else
			{
				m_RootIndex = iB;
			}

			// Rotate
			if (D->Height > E->Height)
			{
				B->SecondChild = iD;
				A->FirstChild = iE;
				E->ParentIndex = iA;
				A->Box = AABB::Union(C->Box, E->Box);
				B->Box = AABB::Union(A->Box, D->Box);

				A->Height = 1 + std::max(C->Height, E->Height);
				B->Height = 1 + std::max(A->Height, D->Height);
			}
			else
			{
				B->SecondChild = iE;
				A->FirstChild = iD;
				D->ParentIndex = iA;
				A->Box = AABB::Union(C->Box, D->Box);
				B->Box = AABB::Union(A->Box, E->Box);

				A->Height = 1 + std::max(C->Height, D->Height);
				B->Height = 1 + std::max(A->Height, E->Height);
			}

			return iB;
		}
		return iA;
	}
}