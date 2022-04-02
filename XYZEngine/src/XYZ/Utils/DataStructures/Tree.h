#pragma once
#include "FreeList.h"


namespace XYZ {

	template <typename T>
	class Tree
	{
	public:
		int32_t Insert(T* data);
		int32_t Insert(T* data, int32_t parent);
		void    Remove(int32_t index);
		void    Clear();


		T*& operator[](int32_t index) { return &m_Nodes[index].Data; }
	private:
		struct Node
		{
			static constexpr int32_t sc_Invalid = -1;

			T*		 Data;
			int32_t  ID				 = sc_Invalid;
			int32_t  Parent			 = sc_Invalid;
			int32_t  FirstChild		 = sc_Invalid;
			int32_t  NextSibling	 = sc_Invalid;
			int32_t  PreviousSibling = sc_Invalid;
			uint32_t Depth			 = 0;
		};
	
		int32_t		   m_Root = Node<T>::sc_Invalid;
		uint32_t	   m_NodeCount = 0;
		FreeList<Node> m_Nodes;
	};

	template<typename T>
	inline int32_t Tree<T>::Insert(T* data)
	{
		Node newNode;
		newNode.Data = data;
		const int32_t newID = m_Nodes.Insert(newNode);
		m_Nodes[newID].ID = newID;

		// Create root
		if (m_Root == Node::sc_Invalid)
		{
			m_Root = newID;
			m_NodeCount++;
			return newID;
		}


		// Find last sibling
		int32_t nextSibling = m_Nodes[m_Root].NextSibling;
		int32_t currentSibling = m_Root;
		while (nextSibling != Node::sc_Invalid)
		{
			currentSibling = nextSibling;
			nextSibling = m_Nodes[currentSibling].NextSibling;
		}

		m_Nodes[currentSibling].NextSibling = newID;
		m_Nodes[newID].PreviousSibling = currentSibling;

		m_NodeCount++;
		return newID;
	}

	template<typename T>
	inline int32_t Tree<T>::Insert(T* data, int32_t parent)
	{
		Node newNode;
		newNode.Data = data;
		const int32_t newID = m_Nodes.Insert(newNode);
		m_Nodes[newID].ID = newID;
		m_Nodes[newID].Parent = parent;

		// Find last sibling
		int32_t nextSibling = m_Nodes[parent].FirstChild;
		int32_t currentSibling = m_Nodes[parent].FirstChild;
		while (nextSibling != Node::sc_Invalid)
		{
			currentSibling = nextSibling;
			nextSibling = m_Nodes[currentSibling].NextSibling;
		}

		// Parent does not have child yet
		if (currentSibling == Node::sc_Invalid)
		{
			m_Nodes[parent].FirstChild = newID;
		}
		else
		{
			m_Nodes[currentSibling].NextSibling = newID;
			m_Nodes[newID].PreviousSibling = currentSibling;
		}

		return newID;
	}

	template<typename T>
	inline void Tree<T>::Remove(int32_t index)
	{
		const Node& removed = m_Nodes[index];
		if (removed.FirstChild != Node::sc_Invalid)
		{
			std::stack<int32_t> stack;
			stack.push(removed.FirstChild);
			while (!stack.empty())
			{
				int32_t current = stack.top();
				stack.pop();

				while (current != Node::sc_Invalid)
				{
					const Node& node = m_Nodes[current];
					const int32_t next = node.NextSibling;
					if (node.FirstChild != Node::sc_Invalid)
						stack.push(node.FirstChild);
					m_NodeCount--;
					m_Nodes.Erase(current);
					current = next;
				}
			}
		}
		if (removed.Parent != Node::sc_Invalid)
		{
			Node& parent = m_Nodes[removed.Parent];
			if (index == parent.FirstChild)
				parent.FirstChild = removed.NextSibling;
		}
		if (removed.NextSibling != Node::sc_Invalid)
		{
			Node& nextSibling = m_Nodes[removed.NextSibling];
			nextSibling.PreviousSibling = removed.PreviousSibling;
		}
		if (removed.PreviousSibling != TreeNode::sc_Invalid)
		{
			Node& previousSibling = m_Nodes[removed.PreviousSibling];
			previousSibling.NextSibling = removed.NextSibling;
		}
		if (index == m_Root)
			m_Root = Node::sc_Invalid;

		m_NodeCount--;
		m_Nodes.Erase(index);
	}

	template<typename T>
	inline void Tree<T>::Clear()
	{
		m_Nodes.Clear();
		m_Root = Node<T>::sc_Invalid;
	}
}