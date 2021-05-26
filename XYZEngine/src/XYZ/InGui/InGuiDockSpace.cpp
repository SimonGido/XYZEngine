#include "stdafx.h"
#include "InGuiDockSpace.h"

#include "InGui.h"

#include <stack>

namespace XYZ {
    InGuiDockNode::InGuiDockNode(InGuiID id)
        :
        ID(id), 
        Parent(nullptr), 
        Sibling(nullptr), 
        Children{nullptr, nullptr}, 
        Split(SplitType::None),
        Position(0.0f),
        Size(0.0f)
    {
    }
    std::pair<InGuiDockNode*, InGuiDockNode*> InGuiDockNode::SplitNode(SplitType split)
    {
        XYZ_ASSERT(Split == SplitType::None, "Node is already split!");
        XYZ_ASSERT(split != SplitType::None, "Split type can not be none");

        Split = split;
        InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;
        
        InGuiDockNode* first = dockSpace.m_Pool.Allocate<InGuiDockNode>(0);
        InGuiDockNode* second = dockSpace.m_Pool.Allocate<InGuiDockNode>(0);
        Children[0] = first;
        Children[1] = second;
        
        first->Sibling = second;
        second->Sibling = first;
        first->Parent = this;
        second->Parent = this;

        first->DockedWindows = std::move(DockedWindows);
        for (auto window : first->DockedWindows)
            window->DockNode = first;
        DockedWindows.clear();
        first->Position = glm::vec2(0.0f);
        if (split == SplitType::Horizontal)
        {
            first->Size = glm::vec2(Size.x, Size.y / 2.0f);
            second->Position =  glm::vec2(0.0f, Size.y / 2.0f);
            second->Size = glm::vec2(Size.x, Size.y / 2.0f);
        }
        else
        {
            first->Size = glm::vec2(Size.x / 2.0f, Size.y);
            second->Position = glm::vec2(Size.x / 2.0f, 0.0f);
            second->Size = glm::vec2(Size.x / 2.0f, Size.y);
        }
        return std::pair<InGuiDockNode*, InGuiDockNode*>(first, second);
    }
    void InGuiDockNode::UnSplit()
    {
        Split = SplitType::None;
        InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;

        DockedWindows = std::move(Children[0]->DockedWindows);
        for (auto window : Children[1]->DockedWindows)
            DockedWindows.push_back(window);

        for (auto window : DockedWindows)
            window->DockNode = this;

        dockSpace.m_Pool.Deallocate<InGuiDockNode>(Children[0]);
        dockSpace.m_Pool.Deallocate<InGuiDockNode>(Children[1]);
        Children[0] = nullptr;
        Children[1] = nullptr;
    }
    glm::vec2 InGuiDockNode::GetCenter() const
    {
        return Position + Size / 2.0f;
    }
    glm::vec2 InGuiDockNode::GetAbsCenter() const
    {
        return GetAbsPosition() + Size / 2.0f;
    }
    glm::vec2 InGuiDockNode::GetAbsPosition() const
    {
        if (Parent)
        {
            return Parent->GetAbsPosition() + Position;
        }
        return Position;
    }
    InGuiRect InGuiDockNode::LeftRect() const
    {
        const InGuiConfig& config = InGui::GetContext().m_Config;
        glm::vec2 center = GetAbsCenter();
        glm::vec2 min(center.x - (config.DockspaceNodeSize.x * 1.5f) - config.DockspaceNodeOffset.x, center.y - config.DockspaceNodeSize.y / 2.0f);
        glm::vec2 max(min + config.DockspaceNodeSize);
        return InGuiRect(min, max);
    }
    InGuiRect InGuiDockNode::RightRect() const
    {
        const InGuiConfig& config = InGui::GetContext().m_Config;
        glm::vec2 center = GetAbsCenter();
        glm::vec2 min(center.x + (config.DockspaceNodeSize.x * 0.5f) + config.DockspaceNodeOffset.x, center.y - config.DockspaceNodeSize.y / 2.0f);
        glm::vec2 max(min + config.DockspaceNodeSize);
        return InGuiRect(min, max);
    }
    InGuiRect InGuiDockNode::TopRect() const
    {
        const InGuiConfig& config = InGui::GetContext().m_Config;
        glm::vec2 center = GetAbsCenter();
        glm::vec2 min(center.x - config.DockspaceNodeSize.x / 2.0f, center.y - (config.DockspaceNodeSize.y * 1.5f) - config.DockspaceNodeOffset.y);
        glm::vec2 max(min + config.DockspaceNodeSize);
        return InGuiRect(min, max);
    }
    InGuiRect InGuiDockNode::BottomRect() const
    {
        const InGuiConfig& config = InGui::GetContext().m_Config;
        glm::vec2 center = GetAbsCenter();
        glm::vec2 min(center.x - config.DockspaceNodeSize.x / 2.0f, center.y + (config.DockspaceNodeSize.y * 0.5f) + config.DockspaceNodeOffset.y);
        glm::vec2 max(min + config.DockspaceNodeSize);
        return InGuiRect(min, max);
    }
    InGuiRect InGuiDockNode::MiddleRect() const
    {
        const InGuiConfig& config = InGui::GetContext().m_Config;
        glm::vec2 center = GetAbsCenter();
        glm::vec2 min(center - config.DockspaceNodeSize / 2.0f);
        glm::vec2 max(min + config.DockspaceNodeSize);
        return InGuiRect(min, max);;
    }
    InGuiDockSpace::InGuiDockSpace()
        :
        m_Root(nullptr),
        m_Pool(20 * sizeof(InGuiDockNode))
    {
    }

    void InGuiDockSpace::Init(const glm::vec2& position, const glm::vec2& size)
    {
        m_Root = m_Pool.Allocate<InGuiDockNode>(0);
        m_Root->Position = position;
        m_Root->Size = size;
    }

    void InGuiDockSpace::Destroy()
    {
        if (m_Root)
        {
            std::stack<InGuiDockNode*> nodes;
            nodes.push(m_Root);
            while (!nodes.empty())
            {
                InGuiDockNode* tmp = nodes.top();
                nodes.pop();

                if (tmp->Split != SplitType::None)
                {
                    nodes.push(tmp->Children[0]);
                    nodes.push(tmp->Children[1]);
                }
                m_Pool.Deallocate<InGuiDockNode>(tmp);
            }
        }
        m_Root = nullptr;
    }
}