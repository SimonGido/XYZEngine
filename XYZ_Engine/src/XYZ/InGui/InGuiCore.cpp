#include "stdafx.h"
#include "InGuiCore.h"

#include "InGuiHelper.h"
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


		static void Generate5SegmentColorRectangle(const glm::vec2& size, Vertex* buffer)
		{
			const uint32_t numSegments = 5;
			uint32_t numVertices = numSegments * 4;
			float segmentSize = size.x / numSegments;

			uint32_t counter = 0;

			float offset = 0.0f;
			while (counter < numVertices)
			{
				buffer[counter].Position = { offset, 0.0f, 0.0f, 1.0f };
				buffer[counter].TexCoord = { 0,0 };

				buffer[counter + 1].Position = { offset + segmentSize,0.0f , 0.0f ,1.0f };
				buffer[counter + 1].TexCoord = { 1,0 };

				buffer[counter + 2].Position = { offset + segmentSize,size.y , 0.0f ,1.0f };
				buffer[counter + 2].TexCoord = { 1,1 };


				buffer[counter + 3].Position = { offset , size.y , 0.0f ,1.0f };
				buffer[counter + 3].TexCoord = { 0,1 };

				offset += segmentSize;
				counter += 4;
			}

			buffer[0].Color = { 1,0,0,1 };
			buffer[1].Color = { 1,1,0,1 };
			buffer[2].Color = { 1,1,0,1 };
			buffer[3].Color = { 1,0,0,1 };
			//////////////////////////////
			buffer[4].Color = { 1,1,0,1 };
			buffer[5].Color = { 0,1,0,1 };
			buffer[6].Color = { 0,1,0,1 };
			buffer[7].Color = { 1,1,0,1 };
			//////////////////////////////
			buffer[8].Color = { 0,1,0,1 };
			buffer[9].Color = { 0,1,1,1 };
			buffer[10].Color = { 0,1,1,1 };
			buffer[11].Color = { 0,1,0,1 };
			//////////////////////////////
			buffer[12].Color = { 0,1,1,1 };
			buffer[13].Color = { 0,0,1,1 };
			buffer[14].Color = { 0,0,1,1 };
			buffer[15].Color = { 0,1,1,1 };
			//////////////////////////////
			buffer[16].Color = { 0,0,1,1 };
			buffer[17].Color = { 1,0,1,1 };
			buffer[18].Color = { 1,0,1,1 };
			buffer[19].Color = { 0,0,1,1 };
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

				
			g_InContext->DockSpace->Begin();

			InGuiRenderer::BeginScene({ g_InContext->FrameData.WindowSize });
			InGuiRenderer::SetMaterial(g_InContext->RenderData.Material);
		}
		void EndFrame()
		{
			g_InContext->DockSpace->End();
			InGuiRenderer::Flush();
			g_InContext->FrameData.CurrentWindow = nullptr;
			
		}

		void OnLeftMouseButtonRelease()
		{
			g_InContext->FrameData.Flags &= ~LeftMouseButtonDown;
			g_InContext->FrameData.Flags &= ~ClickHandled;
			
			g_InContext->FrameData.Flags &= ~WindowBottomResize;
			g_InContext->FrameData.Flags &= ~WindowTopResize;
			g_InContext->FrameData.Flags &= ~WindowRightResize;
			g_InContext->FrameData.Flags &= ~WindowLeftResize;
		}

		void OnRightMouseButtonRelease()
		{
			g_InContext->FrameData.Flags |= DockingHandled;


			g_InContext->FrameData.Flags &= ~RightMouseButtonDown;
			g_InContext->FrameData.Flags &= ~ClickHandled;

			g_InContext->FrameData.Flags &= ~WindowBottomResize;
			g_InContext->FrameData.Flags &= ~WindowTopResize;
			g_InContext->FrameData.Flags &= ~WindowRightResize;
			g_InContext->FrameData.Flags &= ~WindowLeftResize;
		}

		void OnLeftMouseButtonPress()
		{
			g_InContext->FrameData.Flags |= LeftMouseButtonDown;
			g_InContext->FrameData.SelectedPoint = g_InContext->FrameData.MousePosition;
		}

		void OnRightMouseButtonPress()
		{
			g_InContext->FrameData.Flags |= RightMouseButtonDown;
		}

		void OnWindowResize(const glm::vec2& size)
		{
			g_InContext->FrameData.WindowSize = size;	
			g_InContext->DockSpace->FitToWindow(size);
		}

		void OnMouseMove(const glm::vec2& position)
		{
			g_InContext->FrameData.MousePosition = MouseToWorld(position);
		}


		InGuiDockSpace::InGuiDockSpace(const glm::vec2& pos, const glm::vec2& size)
		{
			m_Root = new InGuiDockNode(pos, size);
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
					if (window->DockNode->Parent->Children[0]->Windows.empty()
					 && window->DockNode->Parent->Children[1]->Windows.empty())
					{
						unsplitNode(window->DockNode->Parent);
					}			
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
				g_InContext->DockSpace->ShowDockSpace();
			}
			update(m_Root);
			resize();
		}

		void InGuiDockSpace::resize()
		{
			if (m_ResizedNode)
			{
				if (m_ResizedNode->Split == SplitAxis::Vertical)
				{
					glm::vec2 leftOld = m_ResizedNode->Children[0]->Size;
					glm::vec2 rightOld = m_ResizedNode->Children[1]->Size;
					glm::vec2 leftNew = { g_InContext->FrameData.MousePosition.x - m_ResizedNode->Position.x ,m_ResizedNode->Children[0]->Size.y };
					glm::vec2 rightNew = { (m_ResizedNode->Position.x + m_ResizedNode->Size.x) - m_ResizedNode->Children[1]->Position.x ,m_ResizedNode->Children[1]->Size.y };

					m_ResizedNode->Children[0]->Size.x = leftNew.x;
					m_ResizedNode->Children[1]->Position.x = m_ResizedNode->Position.x + m_ResizedNode->Children[0]->Size.x;
					m_ResizedNode->Children[1]->Size.x = rightNew.x;

					adjustChildrenProps(m_ResizedNode->Children[0]);
					adjustChildrenProps(m_ResizedNode->Children[1]);
				}
				else if (m_ResizedNode->Split == SplitAxis::Horizontal)
				{
					m_ResizedNode->Children[0]->Size.y = g_InContext->FrameData.MousePosition.y - m_ResizedNode->Position.y;
					m_ResizedNode->Children[1]->Position.y = m_ResizedNode->Position.y + m_ResizedNode->Children[0]->Size.y;
					m_ResizedNode->Children[1]->Size.y = (m_ResizedNode->Position.y + m_ResizedNode->Size.y) - m_ResizedNode->Children[1]->Position.y;
					
					adjustChildrenProps(m_ResizedNode->Children[0]);
					adjustChildrenProps(m_ResizedNode->Children[1]);
				}

				if (!(g_InContext->FrameData.Flags & RightMouseButtonDown))
					m_ResizedNode = nullptr;
			}
		}

		void InGuiDockSpace::adjustChildrenProps(InGuiDockNode* node)
		{
			XYZ_ASSERT(node, "Adjusting null node!");
			if (node->Split == SplitAxis::Vertical)
			{
				glm::vec2 halfSize = { node->Size.x / 2, node->Size.y };
				glm::vec2 leftPos = { node->Position.x ,node->Position.y };
				glm::vec2 rightPos = { node->Position.x + halfSize.x,node->Position.y };
				
				node->Children[0]->Position = leftPos;
				node->Children[1]->Position = rightPos;

				node->Children[0]->Size.y = halfSize.y;
				node->Children[1]->Size.y = halfSize.y;
		
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
				node->Children[0]->Size.x = halfSize.x;
				node->Children[1]->Size.x = halfSize.x;
		
				adjustChildrenProps(node->Children[0]);
				adjustChildrenProps(node->Children[1]);
			}
		}

		void InGuiDockSpace::detectResize(InGuiDockNode* node)
		{
			glm::vec2 offset = { 5,5 };
			glm::vec2& mousePos = g_InContext->FrameData.MousePosition;
			if (node->Split != SplitAxis::None)
			{
				if (Collide(node->Position, node->Size, mousePos))
				{
					detectResize(node->Children[0]);
					detectResize(node->Children[1]);
				}
			}
			else
			{
				auto parent = node->Parent;
				if (parent)
				{
					if (parent->Split == SplitAxis::Vertical)
					{
						if (mousePos.x >= parent->Children[0]->Position.x + parent->Children[0]->Size.x - offset.x
							&& mousePos.x <= parent->Children[0]->Position.x + parent->Children[0]->Size.x + offset.x)
						{
							m_ResizedNode = parent;
							g_InContext->FrameData.Flags |= ClickHandled;
						}
					}
					else if (parent->Split == SplitAxis::Horizontal)
					{
						if (mousePos.y >= parent->Children[0]->Position.y + parent->Children[0]->Size.y - offset.y
							&& mousePos.y <= parent->Children[0]->Position.y + parent->Children[0]->Size.y + offset.y)
						{
							m_ResizedNode = parent;
							g_InContext->FrameData.Flags |= ClickHandled;
						}
					}
				}		
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
						splitNode(node, SplitAxis::Vertical);
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
						splitNode(node, SplitAxis::Vertical);
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
						splitNode(node, SplitAxis::Horizontal);
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
						splitNode(node, SplitAxis::Horizontal);
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
				destroy(&((*node)->Children[0]));
			if ((*node)->Children[1])
				destroy(&((*node)->Children[1]));

			for (auto win : (*node)->Windows)
				win->DockNode = nullptr;

			delete *node;
			*node = nullptr;
		}
		void InGuiDockSpace::rescale(const glm::vec2& scale, InGuiDockNode* node)
		{
			node->Size *= scale;
			node->Position *= scale;
			if (node->Children[0])
				rescale(scale, node->Children[0]);
			if (node->Children[1])
				rescale(scale, node->Children[1]);
		}
		void InGuiDockSpace::splitNode(InGuiDockNode* node, SplitAxis axis)
		{
			if (node->Split == SplitAxis::None)
			{
				node->Split = axis;
				if (node->Split == SplitAxis::Vertical)
				{
					glm::vec2 halfSize = { node->Size.x / 2, node->Size.y };
					glm::vec2 leftPos = { node->Position.x ,node->Position.y };
					glm::vec2 rightPos = { node->Position.x + halfSize.x,node->Position.y };
					node->Children[0] = new InGuiDockNode(leftPos, halfSize,node);
					node->Children[1] = new InGuiDockNode(rightPos, halfSize,node);
				}
				else if (node->Split == SplitAxis::Horizontal)
				{
					glm::vec2 halfSize = { node->Size.x ,node->Size.y / 2 };
					glm::vec2 bottomPos = { node->Position.x ,node->Position.y };
					glm::vec2 topPos = { node->Position.x ,node->Position.y + halfSize.y };
					node->Children[0] = new InGuiDockNode(bottomPos, halfSize, node);
					node->Children[1] = new InGuiDockNode(topPos, halfSize, node);
				}
			}
		}

		void InGuiDockSpace::unsplitNode(InGuiDockNode* node)
		{
			if (node)
			{
				node->Split = SplitAxis::None;
				if (node->Children[0])
					destroy(&node->Children[0]);
				if (node->Children[1])
					destroy(&node->Children[1]);
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
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			if (file.read(ini))
			{
				for (auto& it : ini)
				{
					InGuiWindows[it.first] = new InGuiWindow();
					InGuiWindows[it.first]->Position = StringToVec2(it.second.get("position"));
					InGuiWindows[it.first]->Size = StringToVec2(it.second.get("size"));
					if ((bool)atoi(it.second.get("collapsed").c_str()))
						InGuiWindows[it.first]->Flags |= Collapsed;
				}
			}
			else
			{
				file.generate(ini);
			}

			FrameData.WindowSize.x = (float)Input::GetWindowSize().first;
			FrameData.WindowSize.y = (float)Input::GetWindowSize().second;

			DockSpace = new InGuiDockSpace({ -FrameData.WindowSize.x/2,-FrameData.WindowSize.y/2 }, FrameData.WindowSize);
		}
		InGuiContext::~InGuiContext()
		{
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			for (auto& it : g_InContext->InGuiWindows)
			{
				std::string pos = std::to_string(it.second->Position.x) + "," + std::to_string(it.second->Position.y);
				std::string size = std::to_string(it.second->Size.x) + "," + std::to_string(it.second->Size.y);
				ini[it.first]["position"] = pos;
				ini[it.first]["size"] = size;
				bool collapsed = (it.second->Flags & Collapsed);
				ini[it.first]["collapsed"] = std::to_string(collapsed);
				delete it.second;
			}

			file.write(ini);

			if (DockSpace)
			{
				delete DockSpace;
				DockSpace = nullptr;
			}
		}

		InGuiWindow* InGuiContext::GetWindow(const std::string& name)
		{
			auto& it = g_InContext->InGuiWindows.find(name);
			if (it == g_InContext->InGuiWindows.end())
			{
				return nullptr;
			}
			g_InContext->FrameData.CurrentWindow = it->second;
			return it->second;
		}
		InGuiWindow* InGuiContext::CreateWindow(const std::string& name, const glm::vec2& position, const glm::vec2& size)
		{
			g_InContext->InGuiWindows[name] = new InGuiWindow{ position,size };
			g_InContext->FrameData.CurrentWindow = g_InContext->InGuiWindows[name];
			return g_InContext->FrameData.CurrentWindow;
		}
		
		
		
}
}