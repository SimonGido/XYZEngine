#include "stdafx.h"
#include "InGuiStructures.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	InGuiDockSpace::InGuiDockSpace(InGuiDockNode* root)
		:
		m_Root(root)
	{
		m_NodeCount++;
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
			auto it = std::find(window->DockNode->Windows.begin(), window->DockNode->Windows.end(), window);
			if (it != window->DockNode->Windows.end())
				window->DockNode->Windows.erase(it);

			if (window->DockNode->Parent)
			{
				unsplitNode(window->DockNode->Parent);
			}
			window->DockNode = nullptr;
			window->Flags &= ~Docked;
		}
	}
	
	
	void InGuiDockSpace::Begin()
	{

	}

	void InGuiDockSpace::End(const glm::vec2& mousePos, const InGuiRenderConfiguration& renderConfig)
	{
		if (m_DockSpaceVisible)
			showNode(m_Root, mousePos, renderConfig);
		update(m_Root);
		resize(mousePos);
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
		glm::vec2 scale = winSize / m_Root->Size;
		rescale(scale, m_Root);
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
					win->Flags |= Modified;
				for (auto win : m_ResizedNode->Children[1]->Windows)
					win->Flags |= Modified;

				adjustChildrenProps(m_ResizedNode->Children[0]);
				adjustChildrenProps(m_ResizedNode->Children[1]);
			}
			else if (m_ResizedNode->Split == SplitAxis::Horizontal)
			{
				m_ResizedNode->Children[0]->Size.y = mousePos.y - m_ResizedNode->Position.y;
				m_ResizedNode->Children[1]->Position.y = m_ResizedNode->Position.y + m_ResizedNode->Children[0]->Size.y;
				m_ResizedNode->Children[1]->Size.y = (m_ResizedNode->Position.y + m_ResizedNode->Size.y) - m_ResizedNode->Children[1]->Position.y;

				for (auto win : m_ResizedNode->Children[0]->Windows)
					win->Flags |= Modified;
				for (auto win : m_ResizedNode->Children[1]->Windows)
					win->Flags |= Modified;
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
				win->Flags |= Modified;
			for (auto win : node->Children[1]->Windows)
				win->Flags |= Modified;

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
				win->Flags |= Modified;
			for (auto win : node->Children[1]->Windows)
				win->Flags |= Modified;

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
					node->Windows.push_back(window);
					window->DockNode = node;
					window->Flags |= Docked;
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
					node->Children[0]->Windows.push_back(window);
					window->DockNode = node->Children[0];
					window->Flags |= Docked;
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
					node->Children[1]->Windows.push_back(window);
					window->DockNode = node->Children[1];
					window->Flags |= Docked;
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
					node->Children[0]->Windows.push_back(window);
					window->DockNode = node->Children[0];
					window->Flags |= Docked;
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
					node->Children[1]->Windows.push_back(window);
					window->DockNode = node->Children[1];
					window->Flags |= Docked;
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

		m_NodeCount--;
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

				node->Children[0] = new InGuiDockNode(leftPos, firstSize, m_NodeCount, node);
				node->Children[0]->Dock = DockPosition::Left;
				m_NodeCount++;

				node->Children[1] = new InGuiDockNode(rightPos, otherSize, m_NodeCount, node);
				node->Children[1]->Dock = DockPosition::Right;
				m_NodeCount++;
			}
			else if (node->Split == SplitAxis::Horizontal)
			{
				glm::vec2 otherSize = { node->Size.x ,node->Size.y - firstSize.y };
				glm::vec2 bottomPos = { node->Position.x ,node->Position.y };
				glm::vec2 topPos = { node->Position.x ,node->Position.y + otherSize.y };

				node->Children[0] = new InGuiDockNode(bottomPos, firstSize, m_NodeCount, node);
				node->Children[0]->Dock = DockPosition::Bottom;
				m_NodeCount++;

				node->Children[1] = new InGuiDockNode(topPos, otherSize, m_NodeCount, node);
				node->Children[1]->Dock = DockPosition::Top;
				m_NodeCount++;
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
		for (auto win : node->Windows)
		{
			win->Size = { node->Size.x, node->Size.y - InGuiWindow::PanelSize };
			win->Position = node->Position;
		}
		if (node->Children[0])
			update(node->Children[0]);
		if (node->Children[1])
			update(node->Children[1]);
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
	InGuiRenderConfiguration::InGuiRenderConfiguration()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Ref<Texture2D> texture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create("Assets/Font/Arial.fnt");

		Material = Material::Create(shader);
		Material->Set("u_Texture", texture, TextureID);
		Material->Set("u_Texture", Texture2D::Create(TextureWrap::Clamp, "Assets/Font/Arial.png"), FontTextureID);
		Material->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		Material->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));


		ButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		CheckboxSubTextureChecked = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		CheckboxSubTextureUnChecked = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		SliderSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		SliderHandleSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		WindowSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		MinimizeButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		DownArrowButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(3, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		RightArrowButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		DockSpaceSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
	}

	InGuiPerFrameData::InGuiPerFrameData()
	{
		EventReceivingWindow = nullptr;
		ModifiedWindow = nullptr;
		CurrentWindow = nullptr;
		ModifiedWindowMouseOffset = { 0,0 };
		KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		CapslockEnabled = false;

		ResetWindowData();
	}

	void InGuiPerFrameData::ResetWindowData()
	{	
		WindowSpaceOffset = { 0,0 };
		CurrentWindow = nullptr;
		
		MenuBarOffset = { 0,0 };
		PopupOffset = { 0,0 };
		SelectedPoint = { 0,0 };

		MaxHeightInRow = 0.0f;
		MenuItemOffset = 0.0f;
	}
}