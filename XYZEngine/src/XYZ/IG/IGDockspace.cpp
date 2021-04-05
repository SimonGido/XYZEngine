#include "stdafx.h"
#include "IGDockspace.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {


	static uint32_t s_NextID = 0;
	static std::queue<uint32_t> s_FreeIDs;

	namespace Helper {

		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}

		static void ScaleRecursive(IGDockNode* node, const glm::vec2& scale)
		{
			if (node->FirstChild)
				ScaleRecursive(node->FirstChild, scale);
			if (node->SecondChild)
				ScaleRecursive(node->SecondChild, scale);

			node->Data.Size *= scale;
			node->Data.Position *= scale;
		}

		static void UpdateWindowsRecursive(IGDockNode* node)
		{
			if (node->FirstChild)
				UpdateWindowsRecursive(node->FirstChild);
			if (node->SecondChild)
				UpdateWindowsRecursive(node->SecondChild);

			for (auto window : node->Data.Windows)
			{
				window->Position = node->Data.Position;
				window->Size = node->Data.Size;
				if (window->ResizeCallback)
					window->ResizeCallback(window->Size);
			}
		}
		static void DestroyRecursive(IGDockNode* node, MemoryPool& pool)
		{
			if (node->FirstChild)
				DestroyRecursive(node->FirstChild, pool);
			if (node->SecondChild)
				DestroyRecursive(node->SecondChild, pool);

			pool.Deallocate<IGDockNode>(node);
		}

		static bool ResizedBorder(IGSplitType type, const glm::vec2& border, const glm::vec2& mousePos, const glm::vec2& offset)
		{
			if (type == IGSplitType::Vertical)
			{
				if (border.x < mousePos.x + offset.x && border.x > mousePos.x - offset.x)
					return true;
			}
			else if (type == IGSplitType::Horizontal)
			{
				if (border.y < mousePos.y + offset.y && border.y > mousePos.y - offset.y)
					return true;
			}
			return false;
		}


		static bool FindResizeRecursive(IGDockNode* node, IGDockNode** result, const glm::vec2& mousePos)
		{
			if (Collide(node->Data.Position, node->Data.Size, mousePos))
			{
				if (node->FirstChild)
				{
					glm::vec2 border = node->FirstChild->Data.Position + node->FirstChild->Data.Size;
					if (ResizedBorder(node->Type, border, mousePos, { 5.0f,5.0f }))
					{
						*result = node;
						return true;
					}
					if (FindResizeRecursive(node->FirstChild, result, mousePos))
						return true;
				}
				if (node->SecondChild)
				{
					if (FindResizeRecursive(node->SecondChild, result, mousePos))
						return true;
				}
			}
			return false;
		}

		static void SplitNode(IGDockNode* node, IGSplitType type, MemoryPool& pool)
		{
			node->Type = type;
			node->FirstChild =  pool.Allocate<IGDockNode>();
			node->SecondChild = pool.Allocate<IGDockNode>();
			node->FirstChild->Parent = node;
			node->SecondChild->Parent = node;

			if (node->Type == IGSplitType::Vertical)
			{
				node->FirstChild->Data.Position = node->Data.Position;
				node->FirstChild->Data.Size = glm::vec2(node->Data.Size.x / 2.0f, node->Data.Size.y);


				node->SecondChild->Data.Position = glm::vec2(node->Data.Position.x + node->Data.Size.x / 2.0f, node->Data.Position.y);
				node->SecondChild->Data.Size = node->FirstChild->Data.Size;
			}
			else if (node->Type == IGSplitType::Horizontal)
			{
				node->FirstChild->Data.Position = node->Data.Position;
				node->FirstChild->Data.Size = glm::vec2(node->Data.Size.x, node->Data.Size.y / 2.0f);

				node->SecondChild->Data.Position = glm::vec2(node->Data.Position.x, node->Data.Position.y + node->Data.Size.y / 2.0f);
				node->SecondChild->Data.Size = node->FirstChild->Data.Size;
			}
			else
				XYZ_ASSERT(false, "Split Type is None");
		}

		static bool InsertRecursive(IGWindow* window, IGDockNode* node, const glm::vec2& mousePos, MemoryPool& pool)
		{
			glm::vec2 quadSize = glm::vec2(75.0f);
			glm::vec4 quadColor = glm::vec4(1.5f, 1.8f, 2.9f, 1.0f);
			glm::vec2 middlePos = node->Data.Position + ((node->Data.Size - quadSize) / 2.0f);
			glm::vec2 leftPos = node->Data.Position + glm::vec2(0.0f, (node->Data.Size.y - quadSize.y) / 2.0f);
			glm::vec2 rightPos = node->Data.Position + glm::vec2(node->Data.Size.x - quadSize.x, (node->Data.Size.y - quadSize.y) / 2.0f);
			glm::vec2 topPos = node->Data.Position + glm::vec2((node->Data.Size.x - quadSize.x) / 2.0f, 0.0f);
			glm::vec2 bottomPos = node->Data.Position + glm::vec2((node->Data.Size.x - quadSize.x) / 2.0f, node->Data.Size.y - quadSize.y);

			if (node->Type != IGSplitType::None)
			{
				if (Collide(node->Data.Position, node->Data.Size, mousePos))
				{
					if (InsertRecursive(window, node->FirstChild, mousePos, pool))
						return true;
					if (InsertRecursive(window, node->SecondChild, mousePos, pool))
						return true;
				}
			}
			else
			{
				if (Collide(middlePos, quadSize, mousePos))
				{
					node->Data.Windows.push_back(window);
					window->Node = node;
					return true;
				}
				else if (Collide(leftPos, quadSize, mousePos))
				{
					SplitNode(node, IGSplitType::Vertical, pool);
					node->FirstChild->Data.Windows.push_back(window);
					window->Node = node->FirstChild;
					node->SecondChild->Data.Windows = std::move(node->Data.Windows);
					for (auto window : node->SecondChild->Data.Windows)
						window->Node = node->SecondChild;
					node->Data.Windows.clear();
					return true;
				}
				else if (Collide(rightPos, quadSize, mousePos))
				{
					SplitNode(node, IGSplitType::Vertical, pool);
					node->SecondChild->Data.Windows.push_back(window);
					node->FirstChild->Data.Windows = std::move(node->Data.Windows);
					for (auto window : node->FirstChild->Data.Windows)
						window->Node = node->FirstChild;
					node->Data.Windows.clear();
					return true;
				}
				else if (Collide(topPos, quadSize, mousePos))
				{
					SplitNode(node, IGSplitType::Horizontal, pool);
					node->FirstChild->Data.Windows.push_back(window);
					window->Node = node->FirstChild;
					node->SecondChild->Data.Windows = std::move(node->Data.Windows);
					for (auto window : node->SecondChild->Data.Windows)
						window->Node = node->SecondChild;
					node->Data.Windows.clear();
					return true;
				}
				else if (Collide(bottomPos, quadSize, mousePos))
				{
					SplitNode(node, IGSplitType::Horizontal, pool);
					node->SecondChild->Data.Windows.push_back(window);
					window->Node = node->SecondChild;
					node->FirstChild->Data.Windows = std::move(node->Data.Windows);
					for (auto window : node->SecondChild->Data.Windows)
						window->Node = node->SecondChild;
					node->Data.Windows.clear();
					return true;
				}
			}
			return false;
		}

		static void MoveSiblingToParent(IGDockNode* node, MemoryPool& pool)
		{
			IGDockNode* parent = node->Parent;
			if (parent->FirstChild == node)
			{
				parent->Data.Windows = std::move(parent->SecondChild->Data.Windows);
				parent->Type = parent->SecondChild->Type;
				IGDockNode* tmp = parent->SecondChild;
				if (parent->SecondChild->Type != IGSplitType::None)
				{
					tmp->FirstChild->Parent = parent;
					tmp->SecondChild->Parent = parent;
					parent->FirstChild = tmp->FirstChild;
					parent->SecondChild = tmp->SecondChild;
				}
				else
				{
					pool.Deallocate<IGDockNode>(parent->FirstChild);
					parent->FirstChild = nullptr;
					parent->SecondChild = nullptr;
				}
				pool.Deallocate<IGDockNode>(tmp);
			}
			else
			{
				parent->Data.Windows = std::move(parent->FirstChild->Data.Windows);
				for (auto window : parent->Data.Windows)
					window->Node = parent;
				
				parent->Type = parent->FirstChild->Type;
				IGDockNode* tmp = parent->FirstChild;
				if (parent->FirstChild->Type != IGSplitType::None)
				{
					tmp->FirstChild->Parent = parent;
					tmp->SecondChild->Parent = parent;
					parent->FirstChild = tmp->FirstChild;
					parent->SecondChild = tmp->SecondChild;
				}
				else
				{
					pool.Deallocate<IGDockNode>(parent->SecondChild);
					parent->FirstChild = nullptr;
					parent->SecondChild = nullptr;
				}
				pool.Deallocate<IGDockNode>(tmp);
			}
		}
		static void AdjustChildrenRecursive(IGDockNode* node)
		{
			if (node->FirstChild && node->SecondChild)
			{
				if (node->Type == IGSplitType::Vertical)
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

		static void HandleResize(IGDockNode* node, const glm::vec2& mousePos)
		{
			IGDockNode* first = node->FirstChild;
			IGDockNode* second = node->SecondChild;
			if (node->Type == IGSplitType::Vertical)
			{
				first->Data.Size.x = mousePos.x - first->Data.Position.x;
				second->Data.Position.x = first->Data.Position.x + first->Data.Size.x;
				second->Data.Size.x = node->Data.Size.x - first->Data.Size.x;
			}
			else if (node->Type == IGSplitType::Horizontal)
			{
				first->Data.Size.y = mousePos.y - first->Data.Position.y;
				second->Data.Position.y = first->Data.Position.y + first->Data.Size.y;
				second->Data.Size.y = node->Data.Size.y - first->Data.Size.y;
			}

			AdjustChildrenRecursive(first);
			AdjustChildrenRecursive(second);
		}


		static bool RemoveRecursive(IGDockNode* node, IGWindow* window, MemoryPool& pool)
		{
			if (node->FirstChild)
			{
				if (RemoveRecursive(node->FirstChild, window, pool))
					return true;
			}
			if (node->SecondChild)
			{
				if (RemoveRecursive(node->SecondChild, window, pool))
					return true;
			}
			auto it = std::find(node->Data.Windows.begin(), node->Data.Windows.end(), window);
			if (it != node->Data.Windows.end())
			{
				window->Node = nullptr;
				node->Data.Windows.erase(it);
				if (node->Data.Windows.empty() && node->Parent)
				{
					// Unsplit node
					IGDockNode* parent = node->Parent;
					MoveSiblingToParent(node, pool);
					AdjustChildrenRecursive(parent);
				}
				return true;
			}
			return false;
		}

		static void GenerateRecursive(IGDockNode* node, IGMesh* mesh, IGRenderData* renderData)
		{	
			if (node->FirstChild)
				GenerateRecursive(node->FirstChild, mesh, renderData);
			if (node->SecondChild)
				GenerateRecursive(node->SecondChild, mesh, renderData);

			if (node->Type == IGSplitType::None)
			{
				glm::vec4 color = IGRenderData::Colors[IGRenderData::HooverColor];
				color.w = 0.4f;
				IGQuadData data = { 
					node->Data.Position, glm::vec2(75.0f), color,
					renderData->SubTextures[IGRenderData::White], mesh, IGRenderData::TextureID, 0
				};
				IGMeshFactory::GenerateDockNodeQuads(*node, data);
			}
		}

	}

	IGDockNode::IGDockNode()
	{
		if (!s_FreeIDs.empty())
		{
			ID = s_FreeIDs.front();
			s_FreeIDs.pop();
		}
		else
			ID = s_NextID++;
	}

	IGDockNode::IGDockNode(uint32_t id)
	{
		if (s_NextID <= id)
			s_NextID = id + 1;
		ID = id;
	}

	IGDockNode::~IGDockNode()
	{
		s_FreeIDs.push(ID);
	}

	IGDockspace::IGDockspace()
		:
		m_Pool(10 * sizeof(IGDockNode))
	{
		m_Root = m_Pool.Allocate<IGDockNode>();
		m_Root->Data.Position = glm::vec2(0.0f);
		m_Root->Data.Size = glm::vec2(1.0f);
	}

	IGDockspace::IGDockspace(const glm::vec2& pos, const glm::vec2& size)
		:
		m_Pool(10 * sizeof(IGDockNode))
	{
		m_Root = m_Pool.Allocate<IGDockNode>();
		m_Root->Data.Position = pos;
		m_Root->Data.Size = size;
	}
	IGDockspace::~IGDockspace()
	{
		Helper::DestroyRecursive(m_Root, m_Pool);
	}


	void IGDockspace::UpdateWindows()
	{
		Helper::UpdateWindowsRecursive(m_Root);
	}

	void IGDockspace::SetRootSize(const glm::vec2& size)
	{
		glm::vec2 scale = size / m_Root->Data.Size;
		Helper::ScaleRecursive(m_Root, scale);
		Helper::UpdateWindowsRecursive(m_Root);
	}

	void IGDockspace::SubmitToRenderer(IGRenderData* renderData)
	{
		if (m_Visible)
		{
			m_Mesh.Quads.clear();
			m_Mesh.Lines.clear();
			Helper::GenerateRecursive(m_Root, &m_Mesh, renderData);
			for (auto& it : m_Mesh.Quads)
			{
				Renderer2D::SubmitQuadNotCentered(it.Position, it.Size, it.TexCoord, it.TextureID, it.Color);
			}
			for (auto& it : m_Mesh.Lines)
			{
				Renderer2D::SubmitLine(it.P0, it.P1, it.Color);
			}
		}
	}

	bool IGDockspace::InsertWindow(IGWindow* window, const glm::vec2& mousePos)
	{
		bool result = Helper::InsertRecursive(window, m_Root, mousePos, m_Pool);
		if (result)
			Helper::UpdateWindowsRecursive(m_Root);
		return result;
	}
	bool IGDockspace::RemoveWindow(IGWindow* window)
	{
		bool result = Helper::RemoveRecursive(m_Root, window, m_Pool);
		if (result)
			Helper::UpdateWindowsRecursive(m_Root);
		return result;
	}

	bool IGDockspace::OnMouseLeftPress(const glm::vec2& mousePos, bool& handled)
	{
		if (Helper::FindResizeRecursive(m_Root, &m_ResizedNode, mousePos))
			handled = true;
		return handled;
	}

	bool IGDockspace::OnMouseLeftRelease()
	{
		m_ResizedNode = nullptr;
		return false;
	}

	bool IGDockspace::OnMouseMove(const glm::vec2& mousePos, bool& handled)
	{
		if (m_ResizedNode)
		{
			Helper::HandleResize(m_ResizedNode, mousePos);
			Helper::UpdateWindowsRecursive(m_Root);
			handled = true;
			return true;
		}
		return false;
	}
	void IGDockspace::setNextID(uint32_t nextID)
	{
		s_NextID = nextID;
	}
}