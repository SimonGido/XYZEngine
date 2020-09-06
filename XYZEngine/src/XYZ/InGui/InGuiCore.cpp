#include "stdafx.h"
#include "InGuiCore.h"

#include "InGuiHelper.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/InGuiRenderer.h"

#include <ini.h>

namespace XYZ {
	namespace InGui {
#ifndef g_InContext
		InGuiContext* g_InContext = nullptr;
#endif

		static glm::vec2 StringToVec2(const std::string& src)
		{
			glm::vec2 val;
			size_t split = src.find(",", 0);

			val.x = std::stof(src.substr(0, split));
			val.y = std::stof(src.substr(split + 1, src.size() - split));

			return val;
		}

		void Init(const InGuiRenderData& renderData, const InGuiConfig& config)
		{
			g_InContext = new InGuiContext(renderData,config);		
		}

		void Shutdown()
		{
			delete g_InContext;
			g_InContext = nullptr;
		}

		void BeginFrame()
		{
			XYZ_ASSERT(!g_InContext->FrameData.CurrentWindow, "Missing end call for window");
			g_InContext->FrameData.WindowSize.x = Input::GetWindowSize().first;
			g_InContext->FrameData.WindowSize.y = Input::GetWindowSize().second;

			if (!g_InContext->Locked)
				g_InContext->DockSpace->Begin();

			InGuiRenderer::BeginScene({ g_InContext->FrameData.WindowSize });
			InGuiRenderer::SetMaterial(g_InContext->RenderData.Material);
		}
		void EndFrame()
		{
			g_InContext->SubmitToRenderer();
			if (!g_InContext->Locked)
			{
				HandleMouseInput(g_InContext->FrameData.LastActiveWindow);
				HandleMove(g_InContext->FrameData.LastActiveWindow);
				HandleResize(g_InContext->FrameData.LastActiveWindow);
				g_InContext->DockSpace->End();
			}
	
			if (!(g_InContext->FrameData.Flags & RightMouseButtonDown) 
			 && !(g_InContext->FrameData.Flags & LeftMouseButtonDown))
				g_InContext->FrameData.LastActiveWindow = nullptr;
			
			g_InContext->FrameData.CurrentWindow = nullptr;
			g_InContext->FrameData.KeyMode = -1;
			g_InContext->FrameData.PressedKey = -1;
			g_InContext->RenderData.NumTexturesInUse = InGuiRenderData::DefaultTextureCount;

			InGuiRenderer::Flush();
			InGuiRenderer::FlushLines();
			InGuiRenderer::EndScene();
		}

		bool OnLeftMouseButtonRelease()
		{
			g_InContext->FrameData.Flags &= ~LeftMouseButtonDown;
			g_InContext->FrameData.Flags &= ~ClickHandled;
			
			g_InContext->FrameData.Flags &= ~WindowBottomResize;
			g_InContext->FrameData.Flags &= ~WindowTopResize;
			g_InContext->FrameData.Flags &= ~WindowRightResize;
			g_InContext->FrameData.Flags &= ~WindowLeftResize;

			return false;
		}

		bool OnRightMouseButtonRelease()
		{
			g_InContext->FrameData.Flags |= DockingHandled;


			g_InContext->FrameData.Flags &= ~RightMouseButtonDown;
			g_InContext->FrameData.Flags &= ~ClickHandled;

			g_InContext->FrameData.Flags &= ~WindowBottomResize;
			g_InContext->FrameData.Flags &= ~WindowTopResize;
			g_InContext->FrameData.Flags &= ~WindowRightResize;
			g_InContext->FrameData.Flags &= ~WindowLeftResize;

			return false;
		}

		bool OnLeftMouseButtonPress()
		{
			g_InContext->FrameData.Flags |= LeftMouseButtonDown;
			g_InContext->FrameData.SelectedPoint = g_InContext->FrameData.MousePosition;

			for (auto win : g_InContext->GetWindows())
			{
				if ((win.second->Flags & EventReceiver) 
				&& Collide(win.second->Position, win.second->Size, g_InContext->FrameData.MousePosition))
				{
					g_InContext->FrameData.LastActiveWindow = win.second;
					return true;
				}
			}
			return false;
		}

		bool OnRightMouseButtonPress()
		{
			g_InContext->FrameData.Flags |= RightMouseButtonDown;

			for (auto win : g_InContext->GetWindows())
			{
				if ((win.second->Flags & EventReceiver)
					&& Collide(win.second->Position, win.second->Size, g_InContext->FrameData.MousePosition))
				{
					g_InContext->FrameData.LastActiveWindow = win.second;
					return true;
				}
			}
			return false;
		}

		bool OnWindowResize(const glm::vec2& size)
		{
			g_InContext->FrameData.WindowSize = size;	
			g_InContext->DockSpace->FitToWindow(size);

			return false;
		}

		bool OnMouseMove(const glm::vec2& position)
		{
			g_InContext->FrameData.MousePosition = MouseToWorld(position);

			return false;
		}

		bool OnKeyPressed(int key, int mode)
		{
			g_InContext->FrameData.PressedKey = key;
			g_InContext->FrameData.KeyMode = mode;
			if (key == ToUnderlying(KeyCode::XYZ_KEY_CAPS_LOCK))
				g_InContext->FrameData.CapslockEnabled = !g_InContext->FrameData.CapslockEnabled;

			return false;
		}


		void SetLock(bool lock)
		{
			g_InContext->Locked = lock;
		}

		InGuiWindow* GetWindow(const std::string& name)
		{
			return g_InContext->GetWindow(name);
		}


		InGuiDockSpace::InGuiDockSpace(InGuiDockNode* root)
			:
			m_Root(root)
		{
			m_NodeCount++;
		}

		InGuiDockSpace::InGuiDockSpace(const glm::vec2& pos, const glm::vec2& size)
		{
			m_Root = new InGuiDockNode(pos, size, m_NodeCount);;
			m_NodeCount++;
		}
		InGuiDockSpace::~InGuiDockSpace()
		{
			destroy(&m_Root);
		}
		void InGuiDockSpace::FitToWindow(const glm::vec2& size)
		{
			glm::vec2 scale = size / m_Root->Size;
			rescale(scale, m_Root);
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

		void InGuiDockSpace::ShowDockSpace()
		{		
			showNode(m_Root, g_InContext->FrameData.MousePosition);
		}

		void InGuiDockSpace::Begin()
		{		
			if (g_InContext->FrameData.Flags & RightMouseButtonDown
				&& !(g_InContext->FrameData.Flags & ClickHandled))
			{
				detectResize(m_Root);
			}		
		}

		void InGuiDockSpace::End()
		{		
			if (g_InContext->FrameData.Flags & RightMouseButtonDown)
			{
				auto lastActive = g_InContext->FrameData.LastActiveWindow;
				if (lastActive && (lastActive->Flags & Moved))
					g_InContext->DockSpace->ShowDockSpace();

				update(m_Root);
			}
			resize();		
		}

		void InGuiDockSpace::resize()
		{
			if (m_ResizedNode)
			{
				for (auto win : m_ResizedNode->Windows)
					win->Flags |= Modified;
				if (m_ResizedNode->Split == SplitAxis::Vertical)
				{			
					glm::vec2 leftOld = m_ResizedNode->Children[0]->Size;
					glm::vec2 rightOld = m_ResizedNode->Children[1]->Size;
					glm::vec2 leftNew = { g_InContext->FrameData.MousePosition.x - m_ResizedNode->Position.x ,m_ResizedNode->Children[0]->Size.y };
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
					m_ResizedNode->Children[0]->Size.y = g_InContext->FrameData.MousePosition.y - m_ResizedNode->Position.y;
					m_ResizedNode->Children[1]->Position.y = m_ResizedNode->Position.y + m_ResizedNode->Children[0]->Size.y;
					m_ResizedNode->Children[1]->Size.y = (m_ResizedNode->Position.y + m_ResizedNode->Size.y) - m_ResizedNode->Children[1]->Position.y;
					
					for (auto win : m_ResizedNode->Children[0]->Windows)
						win->Flags |= Modified;
					for (auto win : m_ResizedNode->Children[1]->Windows)
						win->Flags |= Modified;
					adjustChildrenProps(m_ResizedNode->Children[0]);
					adjustChildrenProps(m_ResizedNode->Children[1]);
				}

				if (!(g_InContext->FrameData.Flags & RightMouseButtonDown))
				{
					auto& app = Application::Get();
					app.GetWindow().SetCursor(XYZ_ARROW_CURSOR);
					m_ResizedNode = nullptr;
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

		

		void InGuiDockSpace::detectResize(InGuiDockNode* node)
		{
			glm::vec2 offset = { 5,5 };
			glm::vec2& mousePos = g_InContext->FrameData.MousePosition;
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
						g_InContext->FrameData.Flags |= ClickHandled;
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
						g_InContext->FrameData.Flags |= ClickHandled;		
						return;
					}
				}

				detectResize(node->Children[0]);
				detectResize(node->Children[1]);
				
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
						splitNodeProportional(node, SplitAxis::Vertical,halfSize);
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
						splitNodeProportional(node, SplitAxis::Horizontal,halfSize);
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
			delete *node;
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
					glm::vec2 otherSize = { node->Size.x - firstSize.x, node->Size.y};
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
					if (   node->Children[0]->Windows.empty()
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

		void InGuiDockSpace::showNode(InGuiDockNode* node, const glm::vec2& mousePos)
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

					InGuiRenderer::SubmitUI(leftPos, sc_QuadSize, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
					InGuiRenderer::SubmitUI(rightPos, sc_QuadSize, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
					InGuiRenderer::SubmitUI(bottomPos, sc_QuadSize, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
					InGuiRenderer::SubmitUI(topPos, sc_QuadSize, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
					InGuiRenderer::SubmitUI(middlePos, sc_QuadSize, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
				}
				else
				{
					if (node->Children[0])
						showNode(node->Children[0], mousePos);
					if (node->Children[1])
						showNode(node->Children[1], mousePos);
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

	
		InGuiContext::InGuiContext(const InGuiRenderData& renderData, const InGuiConfig& config)
			: RenderData(renderData), ConfigData(config)
		{
			static constexpr uint32_t numPropertiesDockNode = 5;
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			FrameData.WindowSize.x = (float)Input::GetWindowSize().first;
			FrameData.WindowSize.y = (float)Input::GetWindowSize().second;

			if (file.read(ini))
			{
				std::unordered_map<uint32_t, InGuiWindow*> windowMap;
				
				// Load windows
				auto it = ini.begin();
				while (it->first != "dockspace" && it != ini.end())
				{
					m_InGuiWindows[it->first] = new InGuiWindow();
					m_InGuiWindows[it->first]->Name = it->first;
					m_InGuiWindows[it->first]->Position = StringToVec2(it->second.get("position"));
					m_InGuiWindows[it->first]->Size = StringToVec2(it->second.get("size"));
					int32_t id = atoi(it->second.get("docknode").c_str());
					if (id != -1)
						windowMap[id] = m_InGuiWindows[it->first];

					if ((bool)atoi(it->second.get("collapsed").c_str()))
						m_InGuiWindows[it->first]->Flags |= Collapsed;
					
					m_InGuiWindows[it->first]->Flags |= Modified;
					m_InGuiWindows[it->first]->Flags |= EventReceiver;

					it++;
				}


				std::unordered_map<uint32_t, InGuiDockNode*> dockMap;
				std::unordered_map<uint32_t, int32_t> parentMap;
				uint32_t id = 0;

				// Load dockspace
				auto el = it->second.begin();
				while (el != it->second.end())
				{
					std::string nodeID = std::to_string(id);
					glm::vec2 pos = StringToVec2(it->second.get("node position " + nodeID));
					glm::vec2 size = StringToVec2(it->second.get("node size " + nodeID));
					int32_t parentID = atoi(it->second.get("node parent " + nodeID).c_str());
					if (parentID != -1)
						parentMap[id] = parentID;
					
					dockMap[id] = new InGuiDockNode(pos,size, id);
					dockMap[id]->Split = (SplitAxis)atoi(it->second.get("node split " + nodeID).c_str());
					dockMap[id]->Dock =  (DockPosition)atoi(it->second.get("node dockposition " + nodeID).c_str());

					id++;
					el+= numPropertiesDockNode;
				}

				// Setup tree
				for (auto id : parentMap)
				{
					dockMap[id.first]->Parent = dockMap[id.second];
					if (dockMap[id.first]->Dock == DockPosition::Left || dockMap[id.first]->Dock == DockPosition::Bottom)
						dockMap[id.first]->Parent->Children[0] = dockMap[id.first];
					else if (dockMap[id.first]->Dock != DockPosition::None)
						dockMap[id.first]->Parent->Children[1] = dockMap[id.first];
				}
				// Setup windows
				for (auto win : windowMap)
				{
					win.second->Flags |= Docked;
					win.second->DockNode = dockMap[win.first];
					dockMap[win.first]->Windows.push_back(win.second);
				}

				// Setup new dockspace and root
				DockSpace = new InGuiDockSpace(dockMap[0]);
				DockSpace->m_NodeCount = dockMap.size();
			}
			else
			{
				DockSpace = new InGuiDockSpace({ -FrameData.WindowSize.x / 2,-FrameData.WindowSize.y / 2 }, FrameData.WindowSize);
			}
		}


		static void SaveDockSpace(mINI::INIStructure& ini, InGuiDockNode* node)
		{
			std::string nodeID = std::to_string(node->ID);
			ini["Dockspace"]["node position " + nodeID] = std::to_string(node->Position.x) + "," + std::to_string(node->Position.y);
			ini["Dockspace"]["node size " + nodeID] = std::to_string(node->Size.x) + "," + std::to_string(node->Size.y);
			ini["Dockspace"]["node split " + nodeID] = std::to_string(ToUnderlying<SplitAxis>(node->Split));
			ini["Dockspace"]["node dockposition " + nodeID] = std::to_string(ToUnderlying<DockPosition>(node->Dock));

			if (node->Parent)
				ini["Dockspace"]["node parent " + nodeID] = std::to_string(node->Parent->ID);
			else
				ini["Dockspace"]["node parent " + nodeID] = std::to_string(-1);


			if (node->Children[0])
				SaveDockSpace(ini, node->Children[0]);

			if (node->Children[1])
				SaveDockSpace(ini, node->Children[1]);
		}


		InGuiContext::~InGuiContext()
		{
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			file.generate(ini);
			
			for (auto& it : g_InContext->m_InGuiWindows)
			{
				std::string pos = std::to_string(it.second->Position.x) + "," + std::to_string(it.second->Position.y);
				std::string size = std::to_string(it.second->Size.x) + "," + std::to_string(it.second->Size.y);
				ini[it.first]["position"] = pos;
				ini[it.first]["size"] = size;
				bool collapsed = (it.second->Flags & Collapsed);
				ini[it.first]["collapsed"] = std::to_string(collapsed);
				if (it.second->DockNode)
					ini[it.first]["Docknode"] = std::to_string(it.second->DockNode->ID);
				else
					ini[it.first]["Docknode"] = std::to_string(-1);

				delete it.second;
			}


			SaveDockSpace(ini, DockSpace->m_Root);

			file.write(ini);

			if (DockSpace)
			{
				delete DockSpace;
				DockSpace = nullptr;
			}
		}

		InGuiWindow* InGuiContext::GetWindow(const std::string& name)
		{
			auto& it = g_InContext->m_InGuiWindows.find(name);
			if (it == g_InContext->m_InGuiWindows.end())
			{
				return nullptr;
			}
			
			return it->second;
		}
		InGuiWindow* InGuiContext::CreateWin(const std::string& name, const glm::vec2& position, const glm::vec2& size)
		{
			g_InContext->m_InGuiWindows[name] = new InGuiWindow{name, position,size };
			return g_InContext->m_InGuiWindows[name];
		}

		void InGuiContext::SubmitToRenderer()
		{		
			for (auto it : m_InGuiWindows)
			{
				uint8_t priority = 0;
				if (it.second == FrameData.LastActiveWindow)
					priority = 5;

				m_RenderQueue.Push(&it.second->Mesh, priority);
				m_RenderQueue.Push(&it.second->LineMesh);
			}
			m_RenderQueue.SubmitToRenderer();
		}

		void InGuiContext::generateWindow(InGuiWindow* window, const std::string& name)
		{
			glm::vec4 color = ConfigData.DefaultColor;
			glm::vec2 winPos = window->Position;
			glm::vec2 winSize = window->Size;
			glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
			glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

			size_t lastFrameSize = window->Mesh.Vertices.size();
			window->Mesh.TexturePairs.clear();
			window->Mesh.Vertices.clear();
			window->Mesh.Vertices.reserve(lastFrameSize);

			GenerateInGuiQuad(window->Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, RenderData.SliderSubTexture->GetTexCoords(), RenderData.TextureID, color);
			auto [width, height] = GenerateInGuiText(window->Mesh, RenderData.Font, name, panelPos, ConfigData.NameScale, window->Size.x, RenderData.FontTextureID);
			window->MinimalWidth = width + InGuiWindow::PanelSize;
			MoveVertices(window->Mesh.Vertices.data(), { 5, height / 2 }, 4, name.size() * 4);

			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, RenderData.MinimizeButtonSubTexture->GetTexCoords(), RenderData.TextureID);
			if (!(window->Flags & Collapsed))
				GenerateInGuiQuad(window->Mesh, winPos, winSize, RenderData.WindowSubTexture->GetTexCoords(), RenderData.TextureID, color);
		}
		
		
		
}
}