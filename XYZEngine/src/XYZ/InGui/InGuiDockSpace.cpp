#include "stdafx.h"
#include "InGuiDockSpace.h"

#include "InGui.h"

#include <stack>

namespace XYZ {
    InGuiDockNode::InGuiDockNode(InGuiID id)
        :
        ID(id), 
        Parent(nullptr), 
        Children{nullptr, nullptr}, 
        Split(SplitType::None),
        Position(0.0f),
        Size(0.0f)
    {
    }
    InGuiDockNode::~InGuiDockNode()
    {
        InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;
        dockSpace.FreeIDs.push(ID);
    }
    std::pair<InGuiDockNode*, InGuiDockNode*> InGuiDockNode::SplitNode(SplitType split, bool moveWindowsToFirst)
    {
        XYZ_ASSERT(Split == SplitType::None, "Node is already split!");
        XYZ_ASSERT(split != SplitType::None, "Split type can not be none");

        Split = split;
        InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;
        
        InGuiDockNode* first = dockSpace.Pool.Allocate<InGuiDockNode>(dockSpace.GetNextID());
        InGuiDockNode* second = dockSpace.Pool.Allocate<InGuiDockNode>(dockSpace.GetNextID());

        Children[0] = first;
        Children[1] = second;
        
        first->Parent = this;
        second->Parent = this;

        if (moveWindowsToFirst)
        {
            first->DockedWindows = std::move(DockedWindows);
            for (auto window : first->DockedWindows)
                window->DockNode = first;
        }
        else
        {
            second->DockedWindows = std::move(DockedWindows);
            for (auto window : second->DockedWindows)
                window->DockNode = second;
        }

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
        InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;

        DockedWindows = std::move(Children[0]->DockedWindows);
        for (auto window : Children[1]->DockedWindows)
            DockedWindows.push_back(window);

        for (auto window : DockedWindows)
            window->DockNode = this;

        InGuiDockNode* firstChild = Children[0];
        InGuiDockNode* secondChild = Children[1];
        if (Children[0]->Split != SplitType::None)
        {
            firstChild->Children[0]->Parent = this;
            firstChild->Children[1]->Parent = this;
            
            Children[0] = firstChild->Children[0];
            Children[1] = firstChild->Children[1];
            Split = firstChild->Split;
        }
        else if (Children[1]->Split != SplitType::None)
        {
            secondChild->Children[0]->Parent = this;
            secondChild->Children[1]->Parent = this;

            Children[0] = secondChild->Children[0];
            Children[1] = secondChild->Children[1];
            Split = secondChild->Split;
        }
        else
        {
            Children[0] = nullptr;
            Children[1] = nullptr;
            Split = SplitType::None;
        }

        dockSpace.Pool.Deallocate<InGuiDockNode>(firstChild);
        dockSpace.Pool.Deallocate<InGuiDockNode>(secondChild);   
    }
    void InGuiDockNode::Update()
    {
        glm::vec2 absPos = GetAbsPosition();
        for (auto window : DockedWindows)
        {
            window->Position = absPos;
            window->Size = Size;
        }
    }
    void InGuiDockNode::RemoveWindow(InGuiWindow* window)
    {
        auto it = std::find(DockedWindows.begin(), DockedWindows.end(), window);
        if (it != DockedWindows.end())
        {
            DockedWindows.erase(it);
            window->DockNode = nullptr;
            window->Size = window->OriginalSize; // Set back original size
            if (DockedWindows.empty() && Parent)
            {
                Parent->UnSplit();
                return;
            }
        }
    }
    void InGuiDockNode::HandleResize(const glm::vec2& mousePosition)
    {
        glm::vec2 relMousePosition = mousePosition - GetAbsPosition();
        glm::vec2 firstChildOriginalSize = Children[0]->Size;
        glm::vec2 secondChildOriginalSize = Children[1]->Size;

        if (Split == SplitType::Horizontal)
        {
            Children[0]->Size.y = relMousePosition.y - Children[0]->Position.y;
            Children[1]->Position.y = relMousePosition.y;
            Children[1]->Size.y = Size.y - Children[1]->Position.y;
        }
        else
        {
            Children[0]->Size.x = relMousePosition.x - Children[0]->Position.x;
            Children[1]->Position.x = relMousePosition.x;
            Children[1]->Size.x = Size.x - Children[1]->Position.x;
        }
        Children[0]->fitChildren(firstChildOriginalSize);
        Children[1]->fitChildren(secondChildOriginalSize);
    }
    void InGuiDockNode::fitChildren(const glm::vec2& originalSize)
    {
        glm::vec2 ratio = Size / originalSize;
        if (Split == SplitType::Horizontal)
        {
            glm::vec2 firstChildOriginalSize = Children[0]->Size;
            glm::vec2 secondChildOriginalSize = Children[1]->Size;
            Children[0]->Position.x = Position.x;
            Children[1]->Position.x = Position.x;
            Children[0]->Size.x = Size.x;
            Children[1]->Size.x = Size.x;

            Children[0]->Size.y = firstChildOriginalSize.y * ratio.y;
            Children[1]->Position.y = Children[0]->Size.y;
            Children[1]->Size.y = Size.y - Children[1]->Position.y;
            Children[0]->fitChildren(firstChildOriginalSize);
            Children[1]->fitChildren(secondChildOriginalSize);
        }
        else if (Split == SplitType::Vertical)
        {
            glm::vec2 firstChildOriginalSize = Children[0]->Size;
            glm::vec2 secondChildOriginalSize = Children[1]->Size;
            Children[0]->Position.y = Position.y;
            Children[1]->Position.x = Position.y;
            Children[0]->Size.y = Size.y;
            Children[1]->Size.y = Size.y;

            Children[0]->Size.x = firstChildOriginalSize.x * ratio.x;
            Children[1]->Position.x = Children[0]->Size.x;
            Children[1]->Size.x = Size.x - Children[1]->Position.x;
            Children[0]->fitChildren(firstChildOriginalSize);
            Children[1]->fitChildren(secondChildOriginalSize);
        }
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
    InGuiRect InGuiDockNode::Rect() const
    {
        glm::vec2 min(GetAbsPosition());
        return InGuiRect(min, min + Size);
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
        Root(nullptr),
        ResizedNode(nullptr),
        NextID(0)
    {
    }

    void InGuiDockSpace::Init(const glm::vec2& position, const glm::vec2& size)
    {
        Root = Pool.Allocate<InGuiDockNode>(GetNextID());
        Root->Position = position;
        Root->Size = size;
    }

    void InGuiDockSpace::Destroy()
    {
        if (Root)
        {
            std::stack<InGuiDockNode*> nodes;
            nodes.push(Root);
            while (!nodes.empty())
            {
                InGuiDockNode* tmp = nodes.top();
                nodes.pop();

                if (tmp->Split != SplitType::None)
                {
                    nodes.push(tmp->Children[0]);
                    nodes.push(tmp->Children[1]);
                }
                Pool.Deallocate<InGuiDockNode>(tmp);
            }
        }
        Root = nullptr;
    }
    void InGuiDockSpace::Resize(const glm::vec2& size)
    {
        if (Root)
        {
            glm::vec2 scale = size / Root->Size;
            std::stack<InGuiDockNode*> nodes;
            nodes.push(Root);
            while (!nodes.empty())
            {
                InGuiDockNode* tmp = nodes.top();
                nodes.pop();

                if (tmp->Split != SplitType::None)
                {
                    nodes.push(tmp->Children[0]);
                    nodes.push(tmp->Children[1]);
                }
                tmp->Position *= scale;
                tmp->Size *= scale;
            }
        }
    }
    bool InGuiDockSpace::FindResizedNode(const glm::vec2& mousePosition)
    {
        float threshhold = InGui::GetContext().m_Config.ResizeThreshhold;
        std::stack<InGuiDockNode*> nodes;
        nodes.push(Root);
        while (!nodes.empty())
        {
            InGuiDockNode* tmp = nodes.top();
            nodes.pop();

            if (tmp->Split != SplitType::None)
            {               
                InGuiRect firstRect = tmp->Children[0]->Rect();
                InGuiRect secondRect = tmp->Children[1]->Rect();
                firstRect.Max +=  threshhold;
                secondRect.Min -= threshhold;
                firstRect.Union(secondRect);
                if (firstRect.Overlaps(mousePosition))
                {
                    ResizedNode = tmp;
                    return true;
                }
                nodes.push(tmp->Children[0]);
                nodes.push(tmp->Children[1]);
            }
        }
        return false;
    }
    bool InGuiDockSpace::PushNodeRectangle(const InGuiRect& rect)
    {
        const InGuiContext& context = InGui::GetContext();
        const InGuiConfig& config = context.m_Config;
        const InGuiFrame& frame =   context.m_FrameData;
        const InGuiInput& input =   context.m_Input;
        bool result = false;
        glm::vec4 color = config.Colors[InGuiConfig::DockspaceNodeColor];
        if (rect.Overlaps(input.MousePosition))
        {
            color = config.Colors[InGuiConfig::DockspaceNodeHighlight];
            result = true;
        }
        Drawlist.PushQuad(
            color, config.WhiteSubTexture->GetTexCoords(),
            rect.Min, config.DockspaceNodeSize, config.WhiteTextureIndex, 0
        );
        return result;
    }
    bool InGuiDockSpace::InsertWindow(InGuiWindow* window)
    {
        const InGuiContext& context = InGui::GetContext();
        const InGuiConfig& config = context.m_Config;
        const InGuiFrame& frame = context.m_FrameData;
        const InGuiInput& input = context.m_Input;

        // Save original size of the window
        window->OriginalSize = window->Size;

        std::stack<InGuiDockNode*> nodes;
        nodes.push(Root);
        while (!nodes.empty())
        {
            InGuiDockNode* tmp = nodes.top();
            nodes.pop();

            if (tmp->Split != SplitType::None)
            {
                nodes.push(tmp->Children[0]);
                nodes.push(tmp->Children[1]);
            }
            else if (tmp->Rect().Overlaps(input.MousePosition))
            {
                if (tmp->MiddleRect().Overlaps(input.MousePosition))
                {
                    tmp->DockedWindows.push_back(window);
                    window->DockNode = tmp;                   
                    return true;
                }
                if (tmp->LeftRect().Overlaps(input.MousePosition))
                {
                    auto [left, right] = tmp->SplitNode(SplitType::Vertical, false);
                    left->DockedWindows.push_back(window);
                    window->DockNode = left;
                    return true;
                }
                if (tmp->RightRect().Overlaps(input.MousePosition))
                {
                    auto [left, right] = tmp->SplitNode(SplitType::Vertical, true);
                    right->DockedWindows.push_back(window);
                    window->DockNode = right;
                    return true;
                }
                if (tmp->TopRect().Overlaps(input.MousePosition))
                {
                    auto [top, bottom] = tmp->SplitNode(SplitType::Horizontal,false);
                    top->DockedWindows.push_back(window);
                    window->DockNode = top;
                    return true;
                }
                if (tmp->BottomRect().Overlaps(input.MousePosition))
                {
                    auto [top, bottom] = tmp->SplitNode(SplitType::Horizontal, true);
                    bottom->DockedWindows.push_back(window);
                    window->DockNode = bottom;
                    return true;
                }             
            }
        }
        return false;
    }
    InGuiID InGuiDockSpace::GetNextID()
    {
        if (!FreeIDs.empty())
        {
            InGuiID id = FreeIDs.back();
            FreeIDs.pop();
            return id;
        }
        InGuiID id = NextID;
        NextID++;
        return id;
    }
}