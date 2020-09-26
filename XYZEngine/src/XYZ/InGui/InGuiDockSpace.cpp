#include "stdafx.h"
#include "InGuiDockSpace.h"


#include "XYZ/Core/Application.h"

namespace XYZ {
	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}
	static std::pair<int32_t, int32_t> GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
	{
		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;

		int32_t width = 0;
		int32_t height = 0;

		for (auto c : str)
		{
			auto& character = font->GetCharacter(c);
			if (width + (character.XAdvance * scale.x) >= length)
				break;

			float yOffset = (fontData.LineHeight - character.YOffset - character.Height) * scale.y;
			glm::vec2 pos = {
				cursorX + character.XOffset + position.x,
				cursorY + yOffset + position.y
			};

			glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

			mesh.Vertices.push_back({ color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,textureID });
			mesh.Vertices.push_back({ color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,textureID });

			if (size.y > height)
				height = size.y;


			width += character.XAdvance * scale.x;
			cursorX += character.XAdvance * scale.x;
		}
		return std::pair<int32_t, int32_t>(width, height);
	}
	static void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		glm::vec2 texCoords[quadVertexCount] = {
		{texCoord.x,texCoord.y},
		{texCoord.z,texCoord.y},
		{texCoord.z,texCoord.w},
		{texCoord.x,texCoord.w}
		};

		glm::vec4 quadVertexPositions[4] = {
			{ position.x ,		  position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y + size.y, 0.0f, 1.0f },
			{ position.x ,        position.y + size.y, 0.0f, 1.0f }
		};

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			mesh.Vertices.push_back({ color, quadVertexPositions[i], texCoords[i], textureID });
		}
	}
	static void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, size_t offset, size_t count)
	{
		for (size_t i = offset; i < count + offset; ++i)
		{
			vertices[i].Position.x += position.x;
			vertices[i].Position.y += position.y;
		}
	}
	static void GenerateWindowsPanel(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2.0f) - ((float)height / 1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
	}
	InGuiDockSpace::InGuiDockSpace(InGuiDockNode* root)
		:
		m_Root(root)
	{
		m_NextNodeID++;
	}
	InGuiDockSpace::~InGuiDockSpace()
	{
		destroy(&m_Root);
	}

	void InGuiDockSpace::InsertWindow(InGuiWindow* window, const glm::vec2& mousePos)
	{
		insertWindow(window, mousePos, m_Root);
	}

	void InGuiDockSpace::RemoveWindow(InGuiWindow* window)
	{
		if (window->DockNode)
		{
			auto node = window->DockNode;
			auto it = std::find(node->Windows.begin(), node->Windows.end(), window);
			if (it != window->DockNode->Windows.end())
			{
				if (*it == node->VisibleWindow)
					node->VisibleWindow = nullptr;
				node->Windows.erase(it);

				if (node->Windows.size())
				{
					node->VisibleWindow = node->Windows.back();
					node->VisibleWindow->Flags |= Visible;
				}
			}
			if (node->Parent && node->Windows.empty())
			{
				unsplitNode(window->DockNode->Parent);
			}
			window->DockNode = nullptr;
			window->Flags |= Visible;
			window->Flags &= ~Docked;
		}
	}


	void InGuiDockSpace::Begin()
	{

	}

	void InGuiDockSpace::End(const glm::vec2& mousePos, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		if (m_DockSpaceVisible)
			showNode(m_Root, mousePos, renderConfig);
		showNodeWindows(m_Root, mousePos, frameData, renderConfig);
		resize(mousePos);
		update(m_Root);
	}

	bool InGuiDockSpace::OnRightMouseButtonPress(const glm::vec2& mousePos)
	{
		detectResize(m_Root, mousePos);
		if (m_ResizedNode)
			return true;
		return false;
	}

	bool InGuiDockSpace::OnLeftMouseButtonPress()
	{
		return false;
	}

	bool InGuiDockSpace::OnRightMouseButtonRelease(InGuiWindow* window, const glm::vec2& mousePos)
	{
		auto& app = Application::Get();
		app.GetWindow().SetCursor(XYZ_ARROW_CURSOR);
		m_ResizedNode = nullptr;
		m_DockSpaceVisible = false;
		if (window)
			insertWindow(window, mousePos, m_Root);
		return false;
	}

	bool InGuiDockSpace::OnWindowResize(const glm::vec2& winSize)
	{
		if (winSize.x > 1.0f && winSize.y > 1.0f)
		{
			glm::vec2 scale = winSize / m_Root->Size;
			rescale(scale, m_Root);
		}
		return false;
	}


	void InGuiDockSpace::resize(const glm::vec2& mousePos)
	{
		if (m_ResizedNode)
		{
			for (auto win : m_ResizedNode->Windows)
				win->Flags |= Modified;
			if (m_ResizedNode->Split == SplitAxis::Vertical)
			{
				glm::vec2 leftOld = m_ResizedNode->Children[0]->Size;
				glm::vec2 rightOld = m_ResizedNode->Children[1]->Size;
				glm::vec2 leftNew = { mousePos.x - m_ResizedNode->Position.x ,m_ResizedNode->Children[0]->Size.y };
				glm::vec2 rightNew = { (m_ResizedNode->Position.x + m_ResizedNode->Size.x) - m_ResizedNode->Children[1]->Position.x ,m_ResizedNode->Children[1]->Size.y };

				m_ResizedNode->Children[0]->Size.x = leftNew.x;
				m_ResizedNode->Children[1]->Position.x = m_ResizedNode->Position.x + m_ResizedNode->Children[0]->Size.x;
				m_ResizedNode->Children[1]->Size.x = rightNew.x;


				for (auto win : m_ResizedNode->Children[0]->Windows)
				{
					win->Flags |= Modified;
					win->Flags |= Resized;
				}
				for (auto win : m_ResizedNode->Children[1]->Windows)
				{
					win->Flags |= Modified;
					win->Flags |= Resized;
				}

				adjustChildrenProps(m_ResizedNode->Children[0]);
				adjustChildrenProps(m_ResizedNode->Children[1]);
			}
			else if (m_ResizedNode->Split == SplitAxis::Horizontal)
			{
				m_ResizedNode->Children[0]->Size.y = mousePos.y - m_ResizedNode->Position.y;
				m_ResizedNode->Children[1]->Position.y = m_ResizedNode->Position.y + m_ResizedNode->Children[0]->Size.y;
				m_ResizedNode->Children[1]->Size.y = (m_ResizedNode->Position.y + m_ResizedNode->Size.y) - m_ResizedNode->Children[1]->Position.y;

				for (auto win : m_ResizedNode->Children[0]->Windows)
				{
					win->Flags |= Modified;
					win->Flags |= Resized;
				}
				for (auto win : m_ResizedNode->Children[1]->Windows)
				{
					win->Flags |= Modified;
					win->Flags |= Resized;
				}
				adjustChildrenProps(m_ResizedNode->Children[0]);
				adjustChildrenProps(m_ResizedNode->Children[1]);
			}
		}
	}

	void InGuiDockSpace::adjustChildrenProps(InGuiDockNode* node)
	{
		XYZ_ASSERT(node, "Adjusting null node!");
		if (node->Split == SplitAxis::Vertical)
		{
			node->Children[0]->Position = node->Position;
			node->Children[0]->Size.x = node->Children[1]->Position.x - node->Children[0]->Position.x;
			node->Children[0]->Size.y = node->Size.y;
			node->Children[1]->Size.y = node->Size.y;

			for (auto win : node->Children[0]->Windows)
			{
				win->Flags |= Modified;
				win->Flags |= Resized;
			}
			for (auto win : node->Children[1]->Windows)
			{
				win->Flags |= Modified;
				win->Flags |= Resized;
			}

			adjustChildrenProps(node->Children[0]);
			adjustChildrenProps(node->Children[1]);
		}
		else if (node->Split == SplitAxis::Horizontal)
		{
			glm::vec2 halfSize = { node->Size.x ,node->Size.y / 2 };
			glm::vec2 bottomPos = { node->Position.x ,node->Position.y };
			glm::vec2 topPos = { node->Position.x ,node->Position.y + node->Children[0]->Size.y };

			node->Children[0]->Position = bottomPos;
			node->Children[1]->Position = topPos;
			node->Children[0]->Size.x = node->Size.x;
			node->Children[1]->Size.x = node->Size.x;

			for (auto win : node->Children[0]->Windows)
			{
				win->Flags |= Modified;
				win->Flags |= Resized;
			}
			for (auto win : node->Children[1]->Windows)
			{
				win->Flags |= Modified;
				win->Flags |= Resized;
			}

			adjustChildrenProps(node->Children[0]);
			adjustChildrenProps(node->Children[1]);
		}
	}

	void InGuiDockSpace::detectResize(InGuiDockNode* node, const glm::vec2& mousePos)
	{
		glm::vec2 offset = { 5,5 };
		auto& app = Application::Get();

		if (node->Children[0] && node->Children[1])
		{
			if (node->Split == SplitAxis::Vertical)
			{
				if (mousePos.x >= node->Children[0]->Position.x + node->Children[0]->Size.x - offset.x
					&& mousePos.x <= node->Children[0]->Position.x + node->Children[0]->Size.x + offset.x
					&& mousePos.y <= node->Children[0]->Position.y + node->Children[0]->Size.y
					&& mousePos.y >= node->Children[0]->Position.y)
				{
					app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
					m_ResizedNode = node;
					return;
				}
			}
			else if (node->Split == SplitAxis::Horizontal)
			{
				if (mousePos.y >= node->Children[0]->Position.y + node->Children[0]->Size.y - offset.y
					&& mousePos.y <= node->Children[0]->Position.y + node->Children[0]->Size.y + offset.y
					&& mousePos.x <= node->Children[0]->Position.x + node->Children[0]->Size.x
					&& mousePos.x >= node->Children[0]->Position.x)
				{
					app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);
					m_ResizedNode = node;
					return;
				}
			}
			detectResize(node->Children[0], mousePos);
			detectResize(node->Children[1], mousePos);
		}
	}

	void InGuiDockSpace::insertWindow(InGuiWindow* window, const glm::vec2& mousePos, InGuiDockNode* node)
	{
		XYZ_ASSERT(node, "Inserting window to node null!");
		if (Collide(node->Position, node->Size, mousePos))
		{
			if (node->Split == SplitAxis::None)
			{
				auto pos = collideWithMarker(node, mousePos);

				if (pos == DockPosition::Middle)
				{
					XYZ_ASSERT(!window->DockNode, "Window is already docked");

					node->VisibleWindow = window;
					node->Windows.push_back(window);
					window->DockNode = node;
					window->Flags |= Docked;
					window->Flags |= Modified;
				}
				else if (pos == DockPosition::Left)
				{
					XYZ_ASSERT(!window->DockNode, "Window is already docked");
					glm::vec2 halfSize = { node->Size.x / 2, node->Size.y };
					splitNodeProportional(node, SplitAxis::Vertical, halfSize);
					if (!node->Windows.empty())
					{
						for (auto win : node->Windows)
						{
							node->Children[1]->Windows.push_back(win);
							win->DockNode = node->Children[1];
						}
						node->Windows.clear();
					}
					node->Children[0]->VisibleWindow = window;
					node->Children[0]->Windows.push_back(window);
					window->DockNode = node->Children[0];
					window->Flags |= Docked;
					window->Flags |= Modified;
				}
				else if (pos == DockPosition::Right)
				{
					XYZ_ASSERT(!window->DockNode, "Window is already docked");
					glm::vec2 halfSize = { node->Size.x / 2, node->Size.y };
					splitNodeProportional(node, SplitAxis::Vertical, halfSize);
					if (!node->Windows.empty())
					{
						for (auto win : node->Windows)
						{
							node->Children[0]->Windows.push_back(win);
							win->DockNode = node->Children[0];
						}
						node->Windows.clear();
					}

					node->Children[1]->VisibleWindow = window;
					node->Children[1]->Windows.push_back(window);
					window->DockNode = node->Children[1];
					window->Flags |= Docked;
					window->Flags |= Modified;
				}
				else if (pos == DockPosition::Bottom)
				{
					XYZ_ASSERT(!window->DockNode, "Window is already docked");
					glm::vec2 halfSize = { node->Size.x ,node->Size.y / 2 };
					splitNodeProportional(node, SplitAxis::Horizontal, halfSize);
					if (!node->Windows.empty())
					{
						for (auto win : node->Windows)
						{
							node->Children[1]->Windows.push_back(win);
							win->DockNode = node->Children[1];
						}
						node->Windows.clear();
					}

					node->Children[0]->VisibleWindow = window;
					node->Children[0]->Windows.push_back(window);
					window->DockNode = node->Children[0];
					window->Flags |= Docked;
					window->Flags |= Modified;
				}
				else if (pos == DockPosition::Top)
				{
					XYZ_ASSERT(!window->DockNode, "Window is already docked");
					glm::vec2 halfSize = { node->Size.x ,node->Size.y / 2 };
					splitNodeProportional(node, SplitAxis::Horizontal, halfSize);
					if (!node->Windows.empty())
					{
						for (auto win : node->Windows)
						{
							node->Children[0]->Windows.push_back(win);
							win->DockNode = node->Children[0];
						}
						node->Windows.clear();
					}

					node->Children[1]->VisibleWindow = window;
					node->Children[1]->Windows.push_back(window);
					window->DockNode = node->Children[1];
					window->Flags |= Docked;
					window->Flags |= Modified;
				}
			}
			else
			{
				insertWindow(window, mousePos, node->Children[0]);
				insertWindow(window, mousePos, node->Children[1]);
			}
		}
	}
	void InGuiDockSpace::destroy(InGuiDockNode** node)
	{
		if ((*node)->Children[0])
		{
			destroy(&((*node)->Children[0]));
		}
		if ((*node)->Children[1])
		{
			destroy(&((*node)->Children[1]));
		}
		for (auto win : (*node)->Windows)
			win->DockNode = nullptr;

		m_FreeIDs.push((*node)->ID);
		delete* node;
		*node = nullptr;
	}
	void InGuiDockSpace::rescale(const glm::vec2& scale, InGuiDockNode* node)
	{
		node->Size *= scale;
		node->Position *= scale;
		for (auto win : node->Windows)
		{
			win->Size = { node->Size.x,node->Size.y - InGuiWindow::PanelSize };
			win->Position = node->Position;
			win->Flags |= Modified;
		}
		if (node->Children[0])
			rescale(scale, node->Children[0]);
		if (node->Children[1])
			rescale(scale, node->Children[1]);
	}
	void InGuiDockSpace::splitNodeProportional(InGuiDockNode* node, SplitAxis axis, const glm::vec2& firstSize)
	{
		if (node->Split == SplitAxis::None)
		{
			node->Split = axis;
			if (node->Split == SplitAxis::Vertical)
			{
				glm::vec2 otherSize = { node->Size.x - firstSize.x, node->Size.y };
				glm::vec2 leftPos = { node->Position.x ,node->Position.y };
				glm::vec2 rightPos = { node->Position.x + otherSize.x,node->Position.y };

				uint32_t nextID = 0;
				if (!m_FreeIDs.empty())
				{
					nextID = m_FreeIDs.back();
					m_FreeIDs.pop();
				}
				else
					nextID = m_NextNodeID++;

				node->Children[0] = new InGuiDockNode(leftPos, firstSize, nextID, node);
				node->Children[0]->Dock = DockPosition::Left;

				if (!m_FreeIDs.empty())
				{
					nextID = m_FreeIDs.back();
					m_FreeIDs.pop();
				}
				else
					nextID = m_NextNodeID++;

				node->Children[1] = new InGuiDockNode(rightPos, otherSize, nextID, node);
				node->Children[1]->Dock = DockPosition::Right;

			}
			else if (node->Split == SplitAxis::Horizontal)
			{
				glm::vec2 otherSize = { node->Size.x ,node->Size.y - firstSize.y };
				glm::vec2 bottomPos = { node->Position.x ,node->Position.y };
				glm::vec2 topPos = { node->Position.x ,node->Position.y + otherSize.y };

				uint32_t nextID = 0;
				if (!m_FreeIDs.empty())
				{
					nextID = m_FreeIDs.back();
					m_FreeIDs.pop();
				}
				else
					nextID = m_NextNodeID++;
				node->Children[0] = new InGuiDockNode(bottomPos, firstSize, nextID, node);
				node->Children[0]->Dock = DockPosition::Bottom;

				if (!m_FreeIDs.empty())
				{
					nextID = m_FreeIDs.back();
					m_FreeIDs.pop();
				}
				else
					nextID = m_NextNodeID++;
				node->Children[1] = new InGuiDockNode(topPos, otherSize, nextID, node);
				node->Children[1]->Dock = DockPosition::Top;

			}
		}
	}
	void InGuiDockSpace::unsplitNode(InGuiDockNode* node)
	{
		if (node)
		{
			if (node->Children[0] && node->Children[1])
			{
				if (node->Children[0]->Windows.empty()
					&& node->Children[1]->Windows.empty()
					&& node->Children[0]->Split == SplitAxis::None
					&& node->Children[1]->Split == SplitAxis::None)
				{

					node->Split = SplitAxis::None;
					destroy(&node->Children[0]);
					destroy(&node->Children[1]);
				}
			}
			unsplitNode(node->Parent);
		}
	}
	void InGuiDockSpace::update(InGuiDockNode* node)
	{
		if (node->VisibleWindow)
		{
			auto win = node->VisibleWindow;
			win->Size = { node->Size.x, node->Size.y - (2 * InGuiWindow::PanelSize) };
			win->Position = node->Position;
			win->Flags |= Visible;
		}
		if (node->Children[0])
			update(node->Children[0]);
		if (node->Children[1])
			update(node->Children[1]);
	}
	void InGuiDockSpace::showNodeWindows(InGuiDockNode* node, const glm::vec2& mousePos, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		InGuiMesh mesh;
		float widthPerWindow = node->Size.x / node->Windows.size();
		uint32_t counter = 0;
		for (auto win : node->Windows)
		{
			glm::vec2 position = node->Position + glm::vec2{ counter * widthPerWindow ,node->Size.y - InGuiWindow::PanelSize };
			glm::vec2 size = { widthPerWindow,InGuiWindow::PanelSize };
			glm::vec4 color = renderConfig.DefaultColor;

			if (Collide(position, size, mousePos))
			{
				color = renderConfig.HooverColor;
				if ((frameData.Flags & LeftMouseButtonPressed) && !(frameData.Flags & ClickHandled))
				{
					if (node->VisibleWindow)
						node->VisibleWindow->Flags &= ~Visible;

					frameData.Flags |= ClickHandled;
					node->VisibleWindow = win;
					win->Flags |= Modified;
				}
			}
			GenerateWindowsPanel(position, size, color, win->Name, mesh, renderConfig);
			InGuiRenderer::SubmitUI(mesh);
			counter++;
		}
		if (node->Children[0])
			showNodeWindows(node->Children[0], mousePos, frameData, renderConfig);
		if (node->Children[1])
			showNodeWindows(node->Children[1], mousePos, frameData, renderConfig);
	}
	void InGuiDockSpace::showNode(InGuiDockNode* node, const glm::vec2& mousePos, const InGuiRenderConfiguration& renderConfig)
	{
		if (Collide(node->Position, node->Size, mousePos))
		{
			if (node->Split == SplitAxis::None)
			{
				glm::vec2 halfSize = { node->Size.x / 2, node->Size.y / 2 };

				glm::vec2 leftPos = { node->Position.x ,            node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };
				glm::vec2 rightPos = { node->Position.x + node->Size.x - sc_QuadSize.x,node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };
				glm::vec2 bottomPos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2),node->Position.y };
				glm::vec2 topPos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2),node->Position.y + node->Size.y - sc_QuadSize.y };
				glm::vec2 middlePos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2), node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };

				InGuiRenderer::SubmitUI(leftPos, sc_QuadSize, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.SelectColor);
				InGuiRenderer::SubmitUI(rightPos, sc_QuadSize, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.SelectColor);
				InGuiRenderer::SubmitUI(bottomPos, sc_QuadSize, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.SelectColor);
				InGuiRenderer::SubmitUI(topPos, sc_QuadSize, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.SelectColor);
				InGuiRenderer::SubmitUI(middlePos, sc_QuadSize, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.SelectColor);
			}
			else
			{
				if (node->Children[0])
					showNode(node->Children[0], mousePos, renderConfig);
				if (node->Children[1])
					showNode(node->Children[1], mousePos, renderConfig);
			}
		}
	}
	DockPosition InGuiDockSpace::collideWithMarker(InGuiDockNode* node, const glm::vec2& mousePos)
	{
		glm::vec2 halfSize = { node->Size.x / 2, node->Size.y / 2 };

		glm::vec2 leftPos = { node->Position.x ,            node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };
		glm::vec2 rightPos = { node->Position.x + node->Size.x - sc_QuadSize.x,node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };
		glm::vec2 bottomPos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2),node->Position.y };
		glm::vec2 topPos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2),node->Position.y + node->Size.y - sc_QuadSize.y };
		glm::vec2 middlePos = { node->Position.x + halfSize.x - (sc_QuadSize.x / 2), node->Position.y + halfSize.y - (sc_QuadSize.y / 2) };


		if (Collide(leftPos, sc_QuadSize, mousePos))
			return DockPosition::Left;
		if (Collide(rightPos, sc_QuadSize, mousePos))
			return DockPosition::Right;
		if (Collide(topPos, sc_QuadSize, mousePos))
			return DockPosition::Top;
		if (Collide(bottomPos, sc_QuadSize, mousePos))
			return DockPosition::Bottom;
		if (Collide(middlePos, sc_QuadSize, mousePos))
			return DockPosition::Middle;

		return DockPosition::None;
	}
}