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
			g_InContext = new InGuiContext;
			g_InContext->InGuiRenderData = renderData;
			g_InContext->InGuiConfig = config;
			g_InContext->InGuiData.CurrentWindow = nullptr;
			g_InContext->InGuiData.MaxHeightInRow = 0.0f;
			g_InContext->InGuiData.Flags = 0;
		
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			if (file.read(ini))
			{
				for (auto& it : ini)
				{
					g_InContext->InGuiWindows[it.first].Position = StringToVec2(it.second.get("position"));
					g_InContext->InGuiWindows[it.first].Size = StringToVec2(it.second.get("size"));
					if ((bool)atoi(it.second.get("collapsed").c_str()))
						g_InContext->InGuiWindows[it.first].Flags |= Collapsed;
				}
			}
			else
			{
				file.generate(ini);
			}
		}

		void Shutdown()
		{
			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			for (auto& it : g_InContext->InGuiWindows)
			{
				std::string pos = std::to_string(it.second.Position.x) + "," + std::to_string(it.second.Position.y);
				std::string size = std::to_string(it.second.Size.x) + "," + std::to_string(it.second.Size.y);
				ini[it.first]["position"] = pos;
				ini[it.first]["size"] = size;
				bool collapsed = (it.second.Flags & Collapsed);
				ini[it.first]["collapsed"] = std::to_string(collapsed);
			}

			file.write(ini);

			delete g_InContext;
		}

		void BeginFrame()
		{
			XYZ_ASSERT(!g_InContext->InGuiData.CurrentWindow, "Missing end call for window");
			g_InContext->InGuiData.WindowSizeX = Input::GetWindowSize().first;
			g_InContext->InGuiData.WindowSizeY = Input::GetWindowSize().second;
			g_InContext->InGuiData.MousePosition = MouseToWorld({ Input::GetMouseX(),Input::GetMouseY() });

			InGuiRenderer::BeginScene({ {g_InContext->InGuiData.WindowSizeX,g_InContext->InGuiData.WindowSizeY} });
			InGuiRenderer::SetMaterial(g_InContext->InGuiRenderData.Material);
		}
		void EndFrame()
		{
			InGuiRenderer::Flush();
			g_InContext->InGuiData.CurrentWindow = nullptr;
		}

		void OnLeftMouseButtonRelease()
		{
			g_InContext->InGuiData.Flags = 0;
		}

		void OnRightMouseButtonRelease()
		{
			g_InContext->InGuiData.Flags = 0;
		}

		void OnLeftMouseButtonPress()
		{
			g_InContext->InGuiData.Flags |= LeftMouseButtonDown;
		}

		void OnRightMouseButtonPress()
		{
			g_InContext->InGuiData.Flags |= RightMouseButtonDown;
		}

		InGuiFrameData& InGui::GetData()
		{
			return g_InContext->InGuiData;
		}
	}
}