#include "stdafx.h"
#include "Tree.h"


#include <stack>

namespace XYZ {
    Tree::Tree(const Tree& other)
        :
        m_Root(other.m_Root),
        m_Nodes(other.m_Nodes),
        m_NodeCount(other.m_NodeCount)
    {
    }
    Tree::Tree(Tree&& other) noexcept
        :
        m_Root(other.m_Root),
        m_NodeCount(other.m_NodeCount),
        m_Nodes(std::move(other.m_Nodes))
    {
        other.m_Root = TreeNode::sc_Invalid;

    }
    int32_t Tree::Insert(void* data)
    {
        TreeNode newNode;
        newNode.Data = data;
        int32_t newInserted = m_Nodes.Insert(newNode);
        if (m_Root == TreeNode::sc_Invalid)
        {
            m_Root = newInserted;
            m_NodeCount++;
            return newInserted;
        }
        
        int32_t tmpNext = m_Nodes[m_Root].NextSibling;
        if (tmpNext == TreeNode::sc_Invalid)
        {
            m_Nodes[m_Root].NextSibling = newInserted;
            m_Nodes[newInserted].PreviousSibling = m_Root;
        }
        else
        {
            m_Nodes[m_Root].NextSibling = newInserted;
            m_Nodes[newInserted].PreviousSibling = m_Root;
            m_Nodes[newInserted].NextSibling = tmpNext;
            m_Nodes[tmpNext].PreviousSibling = newInserted;
        }
        m_NodeCount++;
        return newInserted;
    }
    int32_t Tree::Insert(void* data, int32_t parent)
    {
        TreeNode newNode;
        newNode.Data = data;
        int32_t newInserted = m_Nodes.Insert(newNode);

        if (m_Root == TreeNode::sc_Invalid)
        {
            m_Root = newInserted;
            m_NodeCount++;
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
        m_Nodes[newInserted].Depth = m_Nodes[parent].Depth + 1;
        parentNode.FirstChild = newInserted;

        m_NodeCount++;
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

                    m_NodeCount--;
                    m_Nodes.Erase(sibling);
                }
                if (m_Nodes[tmp].FirstChild != TreeNode::sc_Invalid)
                    stack.push(m_Nodes[tmp].FirstChild);

                m_NodeCount--;
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
        }
        if (removed.PreviousSibling != TreeNode::sc_Invalid)
        {
            TreeNode& previousSibling = m_Nodes[removed.PreviousSibling];
            previousSibling.NextSibling = removed.NextSibling;
        }
        m_NodeCount--;
        m_Nodes.Erase(index);
    }
    void Tree::ReverseNodeChildren(int32_t node)
    {
        int32_t lastValid = TreeNode::sc_Invalid;
        int32_t tmp = TreeNode::sc_Invalid;
        int32_t current = m_Nodes[node].FirstChild;
        while (current != TreeNode::sc_Invalid)
        {
            tmp = m_Nodes[current].PreviousSibling;
            m_Nodes[current].PreviousSibling = m_Nodes[current].NextSibling;
            m_Nodes[current].NextSibling = tmp;
            lastValid = current;
            current = m_Nodes[current].PreviousSibling;            
        }
        m_Nodes[node].FirstChild = lastValid;
    }
    void Tree::ReverseNode(int32_t node)
    {
        int32_t lastValid = TreeNode::sc_Invalid;
        int32_t current = node;
        int32_t tmp = TreeNode::sc_Invalid;
        while (current != TreeNode::sc_Invalid)
        {
            tmp = m_Nodes[current].PreviousSibling;
            m_Nodes[current].PreviousSibling = m_Nodes[current].NextSibling;
            m_Nodes[current].NextSibling = tmp;
            lastValid = current;
            current = m_Nodes[current].PreviousSibling;
        }
        int32_t parent = m_Nodes[lastValid].Parent;
        if (parent != TreeNode::sc_Invalid)
            m_Nodes[parent].FirstChild = lastValid;
        if (node == m_Root)
            m_Root = lastValid;
    }
    void Tree::Clear()
    {
        m_Nodes.Clear();
        m_NodeCount = 0;
        m_Root = TreeNode::sc_Invalid;
    }
    void Tree::Traverse(const std::function<bool(void*, void*)>& callback) const
    {
        if (m_Root == TreeNode::sc_Invalid)
            return;

        TraverseNode(m_Root, callback);
    }
    void Tree::TraverseNode(int32_t node, const std::function<bool(void*, void*)>& callback) const
    { 
        std::stack<int32_t> stack;
        stack.push(node);
        while (!stack.empty())
        {
            int32_t tmp = stack.top();
            stack.pop();

            const TreeNode& node = m_Nodes[tmp];        
            if (node.NextSibling != TreeNode::sc_Invalid)
                stack.push(node.NextSibling);
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
    void Tree::TraverseNodeChildren(int32_t node, const std::function<bool(void*, void*)>& callback) const
    { 
        uint32_t child = m_Nodes[node].FirstChild;
        while (child != TreeNode::sc_Invalid)
        {
            const TreeNode& node = m_Nodes[child];   
            void* parentData = nullptr;
            void* data = node.Data;
            if (node.Parent != TreeNode::sc_Invalid)
                parentData = m_Nodes[node.Parent].Data;
            if (callback(parentData, data))
                return;
            
            child = m_Nodes[child].NextSibling;
        }
    }

    
    void* Tree::GetParentData(int32_t index)
    {
        if (m_Nodes[index].Parent != TreeNode::sc_Invalid)
        {
            return m_Nodes[m_Nodes[index].Parent].Data;
        }
        return nullptr;
    }
    TreeNode::TreeNode(const TreeNode& other)
        :
        Data(other.Data),
        Parent(other.Parent),
        FirstChild(other.FirstChild),
        NextSibling(other.NextSibling),
        PreviousSibling(other.PreviousSibling),
        Depth(other.Depth)
    {
    }
    TreeNode::TreeNode(TreeNode&& other) noexcept
        :
        Data(other.Data),
        Parent(other.Parent),
        FirstChild(other.FirstChild),
        NextSibling(other.NextSibling),
        PreviousSibling(other.PreviousSibling),
        Depth(other.Depth)
    {
    }
    TreeNode& TreeNode::operator=(const TreeNode& other)
    {
        Data = other.Data;
        Parent = other.Parent;
        FirstChild = other.FirstChild;
        NextSibling = other.NextSibling;
        PreviousSibling = other.PreviousSibling;
        Depth = other.Depth;
        return *this;
    }
}
