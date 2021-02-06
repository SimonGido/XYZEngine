#include "stdafx.h"
#include "InGuiDockspace.h"

#include "InGui.h"
#include "InGuiFactory.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/InGuiRenderer2D.h"

namespace XYZ {

	static InGuiDockNode* s_Root = nullptr;

	static InGuiDockNode* s_ResizedNode = nullptr;

	static InGuiMesh s_Mesh;

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	static void ScaleRecursive(InGuiDockNode* node, const glm::vec2& scale)
	{
		if (node->FirstChild)
			ScaleRecursive(node->FirstChild, scale);
		if (node->SecondChild)
			ScaleRecursive(node->SecondChild, scale);

		node->Data.Size *= scale;
		node->Data.Position *= scale;
	}

	static bool ResizedBorder(InGuiSplitType type, const glm::vec2& border, const glm::vec2& mousePos, const glm::vec2& offset)
	{
		if (type == InGuiSplitType::Vertical)
		{
			if (border.x < mousePos.x + offset.x && border.x > mousePos.x - offset.x)
				return true;
		}
		else if (type == InGuiSplitType::Horizontal)
		{
			if (border.y < mousePos.y + offset.y && border.y > mousePos.y - offset.y)
				return true;
		}
		return false;
	}

	static bool FindResizeRecursive(InGuiDockNode* node, const glm::vec2& mousePos)
	{
		if (Collide(node->Data.Position, node->Data.Size, mousePos))
		{
			if (node->FirstChild)
			{
				glm::vec2 border = node->FirstChild->Data.Position + node->FirstChild->Data.Size;
				if (ResizedBorder(node->Type, border, mousePos, { 5.0f,5.0f }))
				{
					s_ResizedNode = node;
					return true;
				}
				if (FindResizeRecursive(node->FirstChild, mousePos))
					return true;
			}
			if (node->SecondChild)
			{
				if (FindResizeRecursive(node->SecondChild, mousePos))
					return true;
			}
		}
		return false;
	}

	static void DestroyRecursive(InGuiDockNode* node)
	{
		if (node->FirstChild)
			DestroyRecursive(node->FirstChild);
		if (node->SecondChild)
			DestroyRecursive(node->SecondChild);

	
		delete node;
	}

	static void SplitNode(InGuiDockNode& node, InGuiSplitType type)
	{
		node.Type = type;
		node.FirstChild = new InGuiDockNode();
		node.SecondChild = new InGuiDockNode();
		

		node.FirstChild->Parent = &node;
		node.SecondChild->Parent = &node;
		if (node.Type == InGuiSplitType::Vertical)
		{
			node.FirstChild->Data.Position = node.Data.Position;
			node.FirstChild->Data.Size = glm::vec2(node.Data.Size.x / 2.0f, node.Data.Size.y);
			

			node.SecondChild->Data.Position = glm::vec2(node.Data.Position.x + node.Data.Size.x / 2.0f, node.Data.Position.y);
			node.SecondChild->Data.Size = node.FirstChild->Data.Size;
		}
		else if (node.Type == InGuiSplitType::Horizontal)
		{
			node.FirstChild->Data.Position = node.Data.Position;
			node.FirstChild->Data.Size = glm::vec2(node.Data.Size.x, node.Data.Size.y / 2.0f);

			node.SecondChild->Data.Position = glm::vec2(node.Data.Position.x, node.Data.Position.y + node.Data.Size.y / 2.0f);
			node.SecondChild->Data.Size = node.FirstChild->Data.Size;
		}
		else
			XYZ_ASSERT(false, "Split Type is None");
	}

	static bool InsertRecursive(uint32_t windowID, InGuiDockNode& node, const glm::vec2& mousePos)
	{
		glm::vec2 quadSize = glm::vec2(75.0f);
		glm::vec4 quadColor = glm::vec4(1.5f, 1.8f, 2.9f, 1.0f);
		glm::vec2 middlePos = node.Data.Position + ((node.Data.Size - quadSize) / 2.0f);
		glm::vec2 leftPos = node.Data.Position + glm::vec2(0.0f, (node.Data.Size.y - quadSize.y) / 2.0f);
		glm::vec2 rightPos = node.Data.Position + glm::vec2(node.Data.Size.x - quadSize.x, (node.Data.Size.y - quadSize.y) / 2.0f);
		glm::vec2 topPos = node.Data.Position + glm::vec2((node.Data.Size.x - quadSize.x) / 2.0f, 0.0f);
		glm::vec2 bottomPos = node.Data.Position + glm::vec2((node.Data.Size.x - quadSize.x) / 2.0f, node.Data.Size.y - quadSize.y);
		
		if (node.Type != InGuiSplitType::None)
		{
			if (Collide(node.Data.Position, node.Data.Size, mousePos))
			{
				if (InsertRecursive(windowID, *node.FirstChild, mousePos))
					return true;
				if (InsertRecursive(windowID, *node.SecondChild, mousePos))
					return true;
			}
		}
		else
		{
			if (Collide(middlePos, quadSize, mousePos))
			{
				node.Data.Windows.push_back(windowID);
				return true;
			}
			else if (Collide(leftPos, quadSize, mousePos))
			{
				SplitNode(node, InGuiSplitType::Vertical);
				node.FirstChild->Data.Windows.push_back(windowID);
				node.SecondChild->Data.Windows = std::move(node.Data.Windows);
				node.Data.Windows.clear();
				return true;
			}
			else if (Collide(rightPos, quadSize, mousePos))
			{
				SplitNode(node, InGuiSplitType::Vertical);
				node.SecondChild->Data.Windows.push_back(windowID);
				node.FirstChild->Data.Windows = std::move(node.Data.Windows);
				node.Data.Windows.clear();
				return true;
			}
			else if (Collide(topPos, quadSize, mousePos))
			{
				SplitNode(node, InGuiSplitType::Horizontal);
				node.FirstChild->Data.Windows.push_back(windowID);
				node.SecondChild->Data.Windows = std::move(node.Data.Windows);
				node.Data.Windows.clear();
				return true;
			}
			else if (Collide(bottomPos, quadSize, mousePos))
			{
				SplitNode(node, InGuiSplitType::Horizontal);
				node.SecondChild->Data.Windows.push_back(windowID);
				node.FirstChild->Data.Windows = std::move(node.Data.Windows);
				node.Data.Windows.clear();
				return true;
			}
		}
		return false;
	}

	static void MoveSiblingToParent(InGuiDockNode* node)
	{
		InGuiDockNode* parent = node->Parent;
		if (parent->FirstChild == node)
		{
			parent->Data.Windows = std::move(parent->SecondChild->Data.Windows);
			parent->Type = parent->SecondChild->Type;
			InGuiDockNode* tmp = parent->SecondChild;
			if (parent->SecondChild->Type != InGuiSplitType::None)
			{
				tmp->FirstChild->Parent = parent;
				tmp->SecondChild->Parent = parent;
				parent->FirstChild = tmp->FirstChild;
				parent->SecondChild = tmp->SecondChild;
			}
			delete tmp;
		}
		else
		{
			parent->Data.Windows = std::move(parent->FirstChild->Data.Windows);
			parent->Type = parent->FirstChild->Type;
			InGuiDockNode* tmp = parent->FirstChild;
			if (parent->FirstChild->Type != InGuiSplitType::None)
			{
				tmp->FirstChild->Parent = parent;
				tmp->SecondChild->Parent = parent;
				parent->FirstChild = tmp->FirstChild;
				parent->SecondChild = tmp->SecondChild;
			}
			delete tmp;
		}
	}
	static void AdjustChildrenRecursive(InGuiDockNode* node)
	{
		if (node->FirstChild && node->SecondChild)
		{
			if (node->Type == InGuiSplitType::Vertical)
			{
				float oldSize = node->FirstChild->Data.Size.x + node->SecondChild->Data.Size.x;
				float scale = node->Data.Size.x / oldSize;

				node->FirstChild->Data.Position = node->Data.Position;
				node->FirstChild->Data.Size.x = node->SecondChild->Data.Position.x - node->FirstChild->Data.Position.x;
				node->FirstChild->Data.Size.y = node->Data.Size.y;
				node->SecondChild->Data.Position.y = node->Data.Position.y;
				node->SecondChild->Data.Size.y = node->Data.Size.y;
				node->SecondChild->Data.Size.x = node->Data.Size.x - node->FirstChild->Data.Size.x;
			}
			else
			{
				node->FirstChild->Data.Position = node->Data.Position;
				node->FirstChild->Data.Size.y = node->SecondChild->Data.Position.y - node->FirstChild->Data.Position.y;
				node->FirstChild->Data.Size.x = node->Data.Size.x;
				node->SecondChild->Data.Position.x = node->Data.Position.x;
				node->SecondChild->Data.Size.x = node->Data.Size.x;
				node->SecondChild->Data.Size.y = node->Data.Size.y - node->FirstChild->Data.Size.y;
			}
			AdjustChildrenRecursive(node->FirstChild);
			AdjustChildrenRecursive(node->SecondChild);
		}
	}
	static bool RemoveRecursive(InGuiDockNode* node, uint32_t id)
	{
		if (node->FirstChild)
		{
			if (RemoveRecursive(node->FirstChild, id))
				return true;
		}
		if (node->SecondChild)
		{
			if (RemoveRecursive(node->SecondChild, id))
				return true;
		}
		auto it = std::find(node->Data.Windows.begin(), node->Data.Windows.end(), id);
		if (it != node->Data.Windows.end())
		{
			node->Data.Windows.erase(it);
			if (node->Data.Windows.empty() && node->Parent)
			{
				// Unsplit node
				InGuiDockNode* parent = node->Parent;
				MoveSiblingToParent(node);
				AdjustChildrenRecursive(parent);
			}
			return true;
		}
		return false;
	}

	static void GenerateRecursive(InGuiDockNode* node, const glm::vec2& mousePos, const InGuiRenderData& renderData)
	{
		if (Collide(node->Data.Position, node->Data.Size, mousePos))
		{
			if (node->FirstChild)
				GenerateRecursive(node->FirstChild, mousePos, renderData);
			if (node->SecondChild)
				GenerateRecursive(node->SecondChild, mousePos, renderData);

			if (node->Type == InGuiSplitType::None)
				InGuiFactory::GenerateDockNode(*node, s_Mesh, glm::vec2(75.0f), renderData);
		}
	}

	static void SubmitWindowsRecursive(InGuiDockNode* node, InGuiContext* context)
	{
		if (node->FirstChild)
			SubmitWindowsRecursive(node->FirstChild, context);
		if (node->SecondChild)
			SubmitWindowsRecursive(node->SecondChild, context);

		for (uint32_t winIndex : node->Data.Windows)
		{
			InGuiWindow& window = context->Windows[winIndex];
			window.Position = node->Data.Position;
			window.Size = node->Data.Size;

			for (auto it = window.Mesh.Quads.begin(); it != window.Mesh.Quads.end(); ++it)
			{
				InGuiRenderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
			}
			for (auto& line : window.Mesh.Lines)
			{
				InGuiRenderer2D::SubmitLine(line.P0, line.P1, line.Color);
			}
			for (auto it = window.OverlayMesh.Quads.begin(); it != window.OverlayMesh.Quads.end(); ++it)
			{
				InGuiRenderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
			}
			for (auto& line : window.OverlayMesh.Lines)
			{
				InGuiRenderer2D::SubmitLine(line.P0, line.P1, line.Color);
			}
		}
	}

	

	static void HandleResize(InGuiDockNode* node, const glm::vec2& mousePos)
	{
		InGuiDockNode* first = node->FirstChild;
		InGuiDockNode* second = node->SecondChild;
		if (node->Type == InGuiSplitType::Vertical)
		{
			first->Data.Size.x = mousePos.x - first->Data.Position.x;
			second->Data.Position.x = first->Data.Position.x + first->Data.Size.x;
			second->Data.Size.x = node->Data.Size.x - first->Data.Size.x;
		}
		else if (node->Type == InGuiSplitType::Horizontal)
		{
			first->Data.Size.y = mousePos.y - first->Data.Position.y;
			second->Data.Position.y = first->Data.Position.y + first->Data.Size.y;
			second->Data.Size.y = node->Data.Size.y - first->Data.Size.y;
		}

		AdjustChildrenRecursive(first);
		AdjustChildrenRecursive(second);
	}

	void InGuiDockspace::Init(InGuiDockNode* root)
	{
		s_Root = root;
	}

	void InGuiDockspace::Init(const glm::vec2& pos, const glm::vec2& size)
	{
		if (!s_Root)
		{
			s_Root = new InGuiDockNode();
			s_Root->Data.Position = pos;
			s_Root->Data.Size = size;
		}
	}

	void InGuiDockspace::Destroy()
	{
		if (s_Root)
		{
			DestroyRecursive(s_Root);
		}
	}

	void InGuiDockspace::SetRootSize(const glm::vec2& size)
	{
		if (s_Root)
		{
			glm::vec2 scale = size / s_Root->Data.Size;
			ScaleRecursive(s_Root, scale);
		}
	}

	void InGuiDockspace::beginFrame(InGuiContext* context, bool preview)
	{
		if (s_Root)
		{
			if (preview)
				GenerateRecursive(s_Root, context->FrameData.MousePosition, context->RenderData);		
		}
	}

	void InGuiDockspace::endFrame(InGuiContext* context)
	{
		if (s_Root)
		{		
			SubmitWindowsRecursive(s_Root, context);
			if (s_ResizedNode)
				HandleResize(s_ResizedNode, context->FrameData.MousePosition);
			for (auto it = s_Mesh.Quads.begin(); it != s_Mesh.Quads.end(); ++it)
			{
				InGuiRenderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
			}
			for (auto& line : s_Mesh.Lines)
			{
				InGuiRenderer2D::SubmitLine(line.P0, line.P1, line.Color);
			}
			InGuiRenderer2D::Flush();
			InGuiRenderer2D::FlushLines();

			s_Mesh.Lines.clear();
			s_Mesh.Quads.clear();
		}
	}

	bool InGuiDockspace::insertWindow(uint32_t id, const glm::vec2& mousePos)
	{
		if (s_Root)
		{
			return InsertRecursive(id, *s_Root, mousePos);
		}
		return false;
	}
	bool InGuiDockspace::removeWindow(uint32_t id)
	{
		if (s_Root)
		{
			return RemoveRecursive(s_Root, id);
		}
		return false;
	}
	bool InGuiDockspace::onMouseLeftPress(const glm::vec2& mousePos)
	{
		if (s_Root)
		{
			return FindResizeRecursive(s_Root, mousePos);
		}
		return false;
	}
	bool InGuiDockspace::onMouseLeftRelease()
	{
		if (s_Root)
		{
			s_ResizedNode = nullptr;
		}
		return false;
	}
	InGuiDockNode* InGuiDockspace::getRoot()
	{
		return s_Root;
	}
}