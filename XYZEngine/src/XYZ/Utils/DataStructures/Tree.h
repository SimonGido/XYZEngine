#pragma once
#include "FreeList.h"



namespace XYZ {

	struct TreeNode
	{
		static constexpr int32_t sc_Invalid = -1;

		void* Data = nullptr;

		int32_t Parent			= sc_Invalid;
		int32_t FirstChild		= sc_Invalid;
		int32_t NextSibling		= sc_Invalid;
		int32_t PreviousSibling = sc_Invalid;
	};

	class Tree
	{
	public:
		Tree() = default;
		Tree(const Tree& other);

		int32_t Insert(void* data, int32_t parent);
		void    Remove(int32_t index);

		void    Traverse(const std::function<bool(void*, void*)>& callback) const;
		void	SetData(int32_t index, void* data) { m_Nodes[index].Data = data; }
		void*   GetData(int32_t index) { return m_Nodes[index].Data; }
	private:	
		int32_t m_Root = TreeNode::sc_Invalid;
		FreeList<TreeNode> m_Nodes;
	};
}