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

		void EnableDockSpace()
		{
			g_InContext->FrameData.Flags |= DockingEnabled;
			g_InContext->DockSpace.Root[LeftNode] = new InGuiDockNode({ 0,0 }, g_InContext->FrameData.WindowSize);
			g_InContext->DockSpace.Root[RightNode] = new InGuiDockNode({ 0,0 }, { 0,0 });
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

			
			

			InGuiRenderer::BeginScene({ g_InContext->FrameData.WindowSize });
			InGuiRenderer::SetMaterial(g_InContext->RenderData.Material);
		}
		void EndFrame()
		{
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
		}

		void OnMouseMove(const glm::vec2& position)
		{
			g_InContext->FrameData.MousePosition = MouseToWorld(position);
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
		
		
		InGuiDockSpace::InGuiDockSpace()
		{
			Root[LeftNode] = nullptr;
			Root[RightNode] = nullptr;
		}

		InGuiDockSpace::~InGuiDockSpace()
		{
			if (Root[LeftNode])
				destroy(Root[LeftNode]);
			if (Root[RightNode])
				destroy(Root[RightNode]);
		}
		
		void InGuiDockSpace::destroy(InGuiDockNode* node)
		{
			if (node->Children[LeftNode])
				destroy(node->Children[LeftNode]);
			if (node->Children[RightNode])
				destroy(node->Children[RightNode]);

			delete node;
		}
		void InGuiDockNode::InsertWindow(InGuiWindow* window, uint8_t docked)
		{
			
		}
		
}
}