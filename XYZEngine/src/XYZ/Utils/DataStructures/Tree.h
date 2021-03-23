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
		uint32_t Depth = 0;
	};

	class Tree
	{
	public:
		Tree() = default;
		Tree(const Tree& other);

		int32_t     Insert(void* data);
		int32_t     Insert(void* data, int32_t parent);
		void        Remove(int32_t index);
		void        ReverseNodeChildren(int32_t node);
		void        Clear();

		void        Traverse(const std::function<bool(void*, void*)>& callback) const;
		void	    TraverseNode(int32_t node, const std::function<bool(void*, void*)>& callback) const;
		void	    TraverseNodeChildren(int32_t node, const std::function<bool(void*, void*)>& callback) const;
		
		void	    SetData(int32_t index, void* data) { m_Nodes[index].Data = data; }
		void*       GetData(int32_t index) { return m_Nodes[index].Data; }
		const void* GetData(int32_t index) const { return m_Nodes[index].Data; }
		void*	    GetParentData(int32_t index);
		int32_t     GetRoot() const { return m_Root; }
		uint32_t    GetNodeCount() const { return m_NodeCount; }

		const FreeList<TreeNode>& GetFlatNodes() const { return m_Nodes; }
	private:	
		int32_t m_Root = TreeNode::sc_Invalid;
		FreeList<TreeNode> m_Nodes;

		uint32_t m_NodeCount = 0;
	};
}