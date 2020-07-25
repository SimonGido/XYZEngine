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