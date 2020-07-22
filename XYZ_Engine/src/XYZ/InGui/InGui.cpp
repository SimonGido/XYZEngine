#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Core/WindowCodes.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/InGuiRenderer.h"

#include "InGuiHelper.h"

#include <ini.h>

namespace XYZ {

	namespace InGui {
		
		static glm::vec2 StringToVec2(const std::string& src)
		{
			glm::vec2 val;
			size_t split = src.find(",", 0);

			val.x = std::stof(src.substr(0, split));
			val.y = std::stof(src.substr(split + 1, src.size() - split));

			return val;
		}


		void Init(const InGuiRenderData& renderData)
		{
			g_InContext = new InGuiContext;
			g_InContext->InGuiRenderData = renderData;
			g_InContext->InGuiData.CurrentWindow = nullptr;
			g_InContext->InGuiData.MaxHeightInRow = 0.0f;


			mINI::INIFile file("ingui.ini");
			mINI::INIStructure ini;
			if (file.read(ini))
			{
				for (auto& it : ini)
				{
					g_InContext->InGuiWindows[it.first].Position = StringToVec2(it.second.get("position"));
					g_InContext->InGuiWindows[it.first].Size = StringToVec2(it.second.get("size"));
					g_InContext->InGuiWindows[it.first].Collapsed = (bool)atoi(it.second.get("collapsed").c_str());
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
				ini[it.first]["collapsed"] = std::to_string(it.second.Collapsed);
			}

			file.write(ini);

			delete g_InContext;
		}

		void BeginFrame()
		{
			XYZ_ASSERT(!g_InContext->InGuiData.CurrentWindow, "Missing end call for window");
			g_InContext->InGuiData.MousePosition.x = Input::GetMouseX();
			g_InContext->InGuiData.MousePosition.y = Input::GetMouseY();
			g_InContext->InGuiData.WindowSizeX = Input::GetWindowSize().first;
			g_InContext->InGuiData.WindowSizeY = Input::GetWindowSize().second;


			InGuiRenderer::BeginScene({ {g_InContext->InGuiData.WindowSizeX,g_InContext->InGuiData.WindowSizeY} });
			InGuiRenderer::SetMaterial(g_InContext->InGuiRenderData.Material);
		}
		void EndFrame()
		{
			InGuiRenderer::Flush();
			g_InContext->InGuiData.CurrentWindow = nullptr;
		}

		bool Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size, float panelSize)
		{
			if (!g_InContext)
				return false;

			glm::vec4 panelColor = { 1,1,1,1 };
			glm::vec4 color = { 1,1,1,1 };
			glm::vec2 offset = { 10,10 };
			std::string copyName = name;

			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			auto& it = g_InContext->InGuiWindows.find(copyName);
			if (it == g_InContext->InGuiWindows.end())
			{
				g_InContext->InGuiWindows[copyName] = { position,size };
				it = g_InContext->InGuiWindows.find(copyName);
			}

			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - panelSize, panelPos.y };


			// If collide with panel and right button is down set this window as modified
			if (Collide(panelPos, { size.x,panelSize }, g_InContext->InGuiData.MousePosition)
				&& g_InContext->InGuiData.RightMouseButtonDown
				&& !g_InContext->InGuiData.IsWindowModified
				)
			{
				it->second.Modified = true;
				g_InContext->InGuiData.IsWindowModified = true;
				g_InContext->InGuiData.ModifiedWindowMouseOffset = MouseToWorld(g_InContext->InGuiData.MousePosition) - panelPos;
			}
			// If this window is modified modify it
			else if (it->second.Modified)
			{
				// If right mouse button is down
				if (g_InContext->InGuiData.RightMouseButtonDown)
				{
					panelColor = g_InContext->InGuiRenderData.HooverColor;
					panelPos = MouseToWorld(g_InContext->InGuiData.MousePosition) - g_InContext->InGuiData.ModifiedWindowMouseOffset;
					minButtonPos = { panelPos.x + it->second.Size.x - panelSize, panelPos.y };
					it->second.Position = { panelPos.x, panelPos.y - it->second.Size.y };
				}
				// No Longer modified
				else
					it->second.Modified = false;
			}
			// If hoover over window set as active
			else if (Collide(it->second.Position, it->second.Size, g_InContext->InGuiData.MousePosition))
			{
				HandleResize(it->second.Position,it->second.Size, MouseToWorld(g_InContext->InGuiData.MousePosition));
				g_InContext->InGuiData.ActiveWindow = true;
			}
			// Handle colapsing
			else if (Collide(minButtonPos, { panelSize,panelSize }, g_InContext->InGuiData.MousePosition)
				&& g_InContext->InGuiData.LeftMouseButtonDown
				&& !g_InContext->InGuiData.ActiveWidget)
			{
				it->second.Collapsed = !it->second.Collapsed;
				g_InContext->InGuiData.ActiveWidget = true;
			}

			g_InContext->InGuiData.CurrentWindow = &it->second;
			g_InContext->InGuiData.MaxHeightInRow = 0.0f;

			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,panelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, { 0.7,0.7 }, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(panelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, { 1,1,1,1 });

			if (!it->second.Collapsed)
				InGuiRenderer::SubmitUI(it->second.Position, it->second.Size, g_InContext->InGuiRenderData.WindowSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);


			g_InContext->InGuiData.WindowSpaceOffset = { 0, it->second.Size.y };
			return false;
		}

		void End()
		{
			g_InContext->InGuiData.ActiveWindow = false;
		}

		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			if (!g_InContext->InGuiData.CurrentWindow->Collapsed)
			{
				bool pressed = false;
				glm::vec4 color = { 1,1,1,1 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, size.x);
				glm::vec2 textOffset = { (size.x / 2) - (text.Width / 2),(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing(size);

				if (g_InContext->InGuiData.ActiveWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					if (!g_InContext->InGuiData.ActiveWidget)
					{
						pressed = g_InContext->InGuiData.LeftMouseButtonDown;
						g_InContext->InGuiData.ActiveWidget = pressed;
					}
				}

				InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.ButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
				InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);

				return pressed;
			}
			return false;
		}

		bool Checkbox(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			auto it = g_InContext->CacheStates.find(name);
			if (it == g_InContext->CacheStates.end())
			{
				g_InContext->CacheStates[name] = false;
				it = g_InContext->CacheStates.find(name);
			}

			if (!g_InContext->InGuiData.CurrentWindow->Collapsed)
			{
				glm::vec4 color = { 1,1,1,1 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, 1000.0f);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5,size.y });

				if (g_InContext->InGuiData.ActiveWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					if (g_InContext->InGuiData.LeftMouseButtonDown && !g_InContext->InGuiData.ActiveWidget)
					{
						it->second = !it->second;
						g_InContext->InGuiData.ActiveWidget = true;
					}
				}

				if (it->second)
					InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.CheckboxSubTextureChecked->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
				else
					InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.CheckboxSubTextureUnChecked->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);

				InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			}
			return it->second;
		}

		bool Slider(const std::string& name, const glm::vec2& size, float& value)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			if (!g_InContext->InGuiData.CurrentWindow->Collapsed)
			{
				glm::vec4 color = { 1,1,1,1 };
				glm::vec2 handleSize = { size.y, size.y * 2 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, size.x);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5, handleSize.y });

				glm::vec2 handlePos = { position.x + value, position.y - (handleSize.x / 2) };

				bool modified = false;
				if (g_InContext->InGuiData.ActiveWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					modified = g_InContext->InGuiData.LeftMouseButtonDown;
					if (modified && !g_InContext->InGuiData.ActiveWidget)
					{
						float start = position.x;
						value = MouseToWorld(g_InContext->InGuiData.MousePosition).x - start;
					}
				}


				InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
				InGuiRenderer::SubmitUI(handlePos, handleSize, g_InContext->InGuiRenderData.SliderHandleSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);


				InGuiRenderer::SubmitUI(textOffset + position, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);

				return modified;
			}
			return false;
		}

		bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;


			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			if (!g_InContext->InGuiData.CurrentWindow->Collapsed)
			{
				glm::vec4 color = { 1,1,1,1 };
				glm::vec2 position = HandleWindowSpacing(size);
				if (g_InContext->InGuiData.ActiveWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
				}
				InGuiRenderer::SubmitUI(rendererID, position, size, { 0,0,1,1 }, color);
			}
			return false;
		}

		bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
		{
			if (!g_InContext)
				return false;

			glm::vec4 color = { 1,1,1,1 };
			glm::vec4 panelColor = { 1,1,1,1 };
			bool hoover = false;
			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			auto& it = g_InContext->InGuiWindows.find(copyName);
			if (it == g_InContext->InGuiWindows.end())
			{
				g_InContext->InGuiWindows[copyName] = { position,size };
				it = g_InContext->InGuiWindows.find(copyName);
			}

			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - panelSize, panelPos.y };


			// If collide with panel and right button is down set this window as modified
			if (Collide(panelPos, { size.x,panelSize }, g_InContext->InGuiData.MousePosition)
				&& g_InContext->InGuiData.RightMouseButtonDown
				&& !g_InContext->InGuiData.IsWindowModified
				)
			{
				it->second.Modified = true;
				g_InContext->InGuiData.IsWindowModified = true;
				g_InContext->InGuiData.ModifiedWindowMouseOffset = MouseToWorld(g_InContext->InGuiData.MousePosition) - panelPos;
			}
			// If this window is modified modify it
			else if (it->second.Modified)
			{
				// If right mouse button is down
				if (g_InContext->InGuiData.RightMouseButtonDown)
				{
					panelColor = g_InContext->InGuiRenderData.HooverColor;
					panelPos = MouseToWorld(g_InContext->InGuiData.MousePosition) - g_InContext->InGuiData.ModifiedWindowMouseOffset;
					minButtonPos = { panelPos.x + it->second.Size.x - panelSize, panelPos.y };
					it->second.Position = { panelPos.x, panelPos.y - it->second.Size.y };
				}
				// No longer modified
				else
					it->second.Modified = false;
			}
			else if (Collide(it->second.Position, it->second.Size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiRenderData.HooverColor;
				hoover = true;
			}
			// Handle colapsing
			else if (Collide(minButtonPos, { panelSize,panelSize }, g_InContext->InGuiData.MousePosition)
				&& g_InContext->InGuiData.LeftMouseButtonDown
				&& !g_InContext->InGuiData.ActiveWidget)
			{
				it->second.Collapsed = !it->second.Collapsed;
				g_InContext->InGuiData.ActiveWidget = true;
			}


			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,panelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, { 0.7,0.7 }, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(panelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, { 1,1,1,1 });

			if (!it->second.Collapsed)
				InGuiRenderer::SubmitUI(rendererID, it->second.Position, it->second.Size, { 0,0,1,1 }, color);

			return hoover;
		}

		InGuiFrameData& InGui::GetData()
		{
			return g_InContext->InGuiData;
		}
	}
}