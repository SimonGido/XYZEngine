#include "stdafx.h"
#include "Tree.h"


#include <stack>

namespace XYZ {
    Tree::Tree(const Tree& other)
        :
        m_Root(other.m_Root),
        m_Nodes(other.m_Nodes)
    {
    }
    int32_t Tree::Insert(void* data, int32_t parent)
    {
        TreeNode newNode;
        newNode.Data = data;
        int32_t newInserted = m_Nodes.Insert(newNode);

        if (m_Root == TreeNode::sc_Invalid)
        {
            m_Root = newInserted;
            return newInserted;
        }     
        TreeNode& parentNode = m_Nodes[parent];
        if (parentNode.FirstChild != TreeNode::sc_Invalid)
        {
            int32_t tmp = parentNode.FirstChild;
            m_Nodes[newInserted].NextSibling = tmp;
            m_Nodes[tmp].PreviousSibling = newInserted;
        }
      
        m_Nodes[newInserted].Parent = parent;
        parentNode.FirstChild = newInserted;

        return newInserted;
    }
    void Tree::Remove(int32_t index)
    {
        TreeNode& removed = m_Nodes[index];
        if (removed.FirstChild != TreeNode::sc_Invalid)
        {
            std::stack<int32_t> stack;
            stack.push(removed.FirstChild);
            while (!stack.empty())
            {
                int32_t tmp = stack.top();
                stack.pop();

                int32_t sibling = m_Nodes[tmp].NextSibling;
                while (sibling != TreeNode::sc_Invalid)
                {
                    TreeNode& siblingNode = m_Nodes[sibling];
                    if (siblingNode.FirstChild != TreeNode::sc_Invalid)
                        stack.push(siblingNode.FirstChild);
                    m_Nodes.Erase(sibling);
                }
                if (m_Nodes[tmp].FirstChild != TreeNode::sc_Invalid)
                    stack.push(m_Nodes[tmp].FirstChild);

                m_Nodes.Erase(tmp);
            }
        }

        if (removed.Parent != TreeNode::sc_Invalid)
        {
            TreeNode& parent = m_Nodes[removed.Parent];
            if (index == parent.FirstChild)
                parent.FirstChild = removed.NextSibling;
        }
        if (removed.NextSibling != TreeNode::sc_Invalid)
        {
            TreeNode& nextSibling = m_Nodes[removed.NextSibling];
            nextSibling.PreviousSibling = removed.PreviousSibling;
            if (removed.PreviousSibling != TreeNode::sc_Invalid)
                m_Nodes[removed.PreviousSibling].NextSibling = removed.NextSibling;
        }
        m_Nodes.Erase(index);
    }
 
    void Tree::Traverse(const std::function<bool(void*, void*)>& callback) const
    {
        if (m_Root == TreeNode::sc_Invalid)
            return;

        std::stack<int32_t> stack;
        stack.push(m_Root);
        while (!stack.empty())
        {
            int32_t tmp = stack.top();
            stack.pop();
            const TreeNode& node = m_Nodes[tmp];
           
            int32_t sibling = node.NextSibling;
            while (sibling != TreeNode::sc_Invalid)
            {
                const TreeNode& siblingNode = m_Nodes[sibling];
                if (siblingNode.FirstChild != TreeNode::sc_Invalid)
                    stack.push(siblingNode.FirstChild);

                void* parentData = nullptr;
                void* data = siblingNode.Data;     
                if (node.Parent != TreeNode::sc_Invalid)
                    parentData = m_Nodes[node.Parent].Data;
                if (callback(parentData, data))
                    return;

                sibling = siblingNode.NextSibling;
            }
            if (node.FirstChild != TreeNode::sc_Invalid)
                stack.push(node.FirstChild);
            
            void* parentData = nullptr;
            void* data = node.Data;
            if (node.Parent != TreeNode::sc_Invalid)
                parentData = m_Nodes[node.Parent].Data;
            if (callback(parentData, data))
                return;
        }
    }
}
