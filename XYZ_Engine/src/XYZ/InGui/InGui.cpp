#include "stdafx.h"
#include "InGui.h"


#include "XYZ/Core/Application.h"
#include "XYZ/Core/WindowCodes.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/InGuiRenderer.h"

#include "InGuiHelper.h"


namespace XYZ {

	namespace InGui {
		
		bool Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size)
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

			HandleMouseInput(it->second);
			
			if (it->second.Flags & Resized)
			{
				HandleResize(it->second);
			}
			// If this window is moving modify it
			else if (it->second.Flags & Moved)
			{
				HandleMove(it->second);
				panelColor = g_InContext->InGuiRenderData.HooverColor;
			}
		
			g_InContext->InGuiData.CurrentWindow = &it->second;
			g_InContext->InGuiData.MaxHeightInRow = 0.0f;
			
			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - InGuiWindow::PanelSize, panelPos.y };
			
			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,InGuiWindow::PanelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, { 0.7,0.7 }, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(InGuiWindow::PanelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, { 1,1,1,1 });

			if (!(it->second.Flags & Collapsed))
				InGuiRenderer::SubmitUI(it->second.Position, it->second.Size, g_InContext->InGuiRenderData.WindowSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);


			g_InContext->InGuiData.WindowSpaceOffset = { 0, it->second.Size.y };
			return false;
		}

		void End()
		{
		}

		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			if (!(g_InContext->InGuiData.CurrentWindow->Flags & Collapsed))
			{
				bool pressed = false;
				glm::vec4 color = { 1,1,1,1 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, size.x);
				glm::vec2 textOffset = { (size.x / 2) - (text.Width / 2),(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing(size);

				bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
				if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					if (!g_InContext->InGuiData.ClickHandled)
					{
						pressed = g_InContext->InGuiData.LeftMouseButtonDown;
						g_InContext->InGuiData.ClickHandled = pressed;
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

			if (!(g_InContext->InGuiData.CurrentWindow->Flags & Collapsed))
			{
				glm::vec4 color = { 1,1,1,1 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, 1000.0f);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5,size.y });

				bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
				if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					if (g_InContext->InGuiData.LeftMouseButtonDown && !g_InContext->InGuiData.ClickHandled)
					{
						it->second = !it->second;
						g_InContext->InGuiData.ClickHandled = true;
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
			if (!(g_InContext->InGuiData.CurrentWindow->Flags & Collapsed))
			{
				glm::vec4 color = { 1,1,1,1 };
				glm::vec2 handleSize = { size.y, size.y * 2 };

				InGuiText text;
				GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, { 0.7,0.7 }, size.x);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5, handleSize.y });

				glm::vec2 handlePos = { position.x + value, position.y - (handleSize.x / 2) };

				bool modified = false;
				bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
				if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
				{
					color = g_InContext->InGuiRenderData.HooverColor;
					modified = g_InContext->InGuiData.LeftMouseButtonDown;
					if (modified && !g_InContext->InGuiData.ClickHandled)
					{
						float start = position.x;
						value = g_InContext->InGuiData.MousePosition.x - start;
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
			if (!(g_InContext->InGuiData.CurrentWindow->Flags & Collapsed))
			{
				glm::vec4 color = { 1,1,1,1 };
				glm::vec2 position = HandleWindowSpacing(size);
				bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
				if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
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
			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			auto& it = g_InContext->InGuiWindows.find(copyName);
			if (it == g_InContext->InGuiWindows.end())
			{
				g_InContext->InGuiWindows[copyName] = { position,size };
				it = g_InContext->InGuiWindows.find(copyName);
			}
	
			HandleMouseInput(it->second);

			if (it->second.Flags & Resized)
			{
				HandleResize(it->second);
			}
			// If this window is moving modify it
			else if (it->second.Flags & Moved)
			{
				HandleMove(it->second);
				panelColor = g_InContext->InGuiRenderData.HooverColor;
			}

			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - InGuiWindow::PanelSize, panelPos.y };

			//HandleResize(it->second);
			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,panelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, { 0.7,0.7 }, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(panelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, { 1,1,1,1 });

			if (!(it->second.Flags & Collapsed))
				InGuiRenderer::SubmitUI(rendererID, it->second.Position, it->second.Size, { 0,0,1,1 }, color);

			return (it->second.Flags & Hoovered);
		}

	}
}