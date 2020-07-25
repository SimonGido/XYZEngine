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

			glm::vec4 panelColor = g_InContext->InGuiConfig.DefaultColor;
			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;

			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			auto& it = g_InContext->InGuiWindows.find(copyName);
			if (it == g_InContext->InGuiWindows.end())
			{
				g_InContext->InGuiWindows[copyName] = { position,size };
				it = g_InContext->InGuiWindows.find(copyName);
			}
			g_InContext->InGuiData.CurrentWindow = &it->second;
			g_InContext->InGuiData.MaxHeightInRow = 0.0f;
			g_InContext->InGuiData.WindowSpaceOffset = { 0, it->second.Size.y };

			HandleMouseInput(it->second);	
			if (it->second.Flags & Resized)
			{
				HandleResize(it->second);
			}
			// If this window is moving modify it
			else if (it->second.Flags & Moved)
			{
				HandleMove(it->second);
				panelColor = g_InContext->InGuiConfig.HooverColor;
			}	
			
			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - InGuiWindow::PanelSize, panelPos.y };
			
			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,InGuiWindow::PanelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, g_InContext->InGuiConfig.NameScale, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(InGuiWindow::PanelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, { 1,1,1,1 });

			if (!(it->second.Flags & Collapsed))
				InGuiRenderer::SubmitUI(it->second.Position, it->second.Size, g_InContext->InGuiRenderData.WindowSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);


			return !(it->second.Flags & Collapsed);
		}

		void End()
		{
			g_InContext->InGuiData.CurrentWindow = nullptr;
		}


		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			
			bool pressed = false;
			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;

			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, g_InContext->InGuiConfig.NameScale, size.x);
			glm::vec2 textOffset = { (size.x / 2) - (text.Width / 2),(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing(size);

			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiConfig.HooverColor;
				if (!(g_InContext->InGuiData.Flags & ClickHandled))
				{
					pressed = g_InContext->InGuiData.Flags & LeftMouseButtonDown;
					if (pressed)
						g_InContext->InGuiData.Flags |= ClickHandled;
				}
			}

			InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.ButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
			InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);

			return pressed;
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

			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;

			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, g_InContext->InGuiConfig.NameScale, g_InContext->InGuiConfig.MaxTextLength);
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5,size.y });

			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiConfig.HooverColor;
				if ((g_InContext->InGuiData.Flags & LeftMouseButtonDown) 
				&& !(g_InContext->InGuiData.Flags & ClickHandled))
				{
					it->second = !it->second;
					g_InContext->InGuiData.Flags |= ClickHandled;
				}
			}

			if (it->second)
				InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.CheckboxSubTextureChecked->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
			else
				InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.CheckboxSubTextureUnChecked->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);

			InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
		
			return it->second;
		}

		bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
		
			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;
			glm::vec2 handleSize = { size.y, size.y * 2 };

			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, {}, g_InContext->InGuiConfig.NameScale, size.x);
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5, handleSize.y });

			glm::vec2 handlePos = { position.x + (value * valueScale) - handleSize.x/2, position.y - (handleSize.x / 2) };

			bool modified = false;
			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiConfig.HooverColor;
				modified = g_InContext->InGuiData.Flags & LeftMouseButtonDown;
				if (modified && !(g_InContext->InGuiData.Flags & ClickHandled))
				{
					float start = position.x;
					value = (g_InContext->InGuiData.MousePosition.x - start) / valueScale;
				}
			}


			InGuiRenderer::SubmitUI(position, size, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);
			InGuiRenderer::SubmitUI(handlePos, handleSize, g_InContext->InGuiRenderData.SliderHandleSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, color);


			InGuiRenderer::SubmitUI(textOffset + position, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);

			return modified;
		}

		bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");

			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size);
			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiConfig.HooverColor;
			}
			InGuiRenderer::SubmitUI(rendererID, position, size, { 0,0,1,1 }, color);
			
			return false;
		}

		bool Text(const std::string& text, const glm::vec2& scale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");
			
			InGuiText txt;
			GenerateInGuiText(txt, g_InContext->InGuiRenderData.Font, text, {}, scale, g_InContext->InGuiConfig.MaxTextLength);
			glm::vec2 size = { txt.Width, txt.Height };
			glm::vec2 position = HandleWindowSpacing(size);
			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;

			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				color = g_InContext->InGuiConfig.HooverColor;
			}

			InGuiRenderer::SubmitUI(position, txt.Vertices.data(), txt.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			
			return false;
		}

		bool ColorPicker4(const std::string& name, const glm::vec2 & size, const glm::vec4& pallete, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			glm::vec2 position = HandleWindowSpacing(size);
			
			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");

			bool modified = false;
			bool activeWindow = g_InContext->InGuiData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->InGuiData.MousePosition))
			{
				modified = g_InContext->InGuiData.Flags & LeftMouseButtonDown;
				if (modified)
				{				
					color = CalculatePixelColor(pallete, position, size);
				}
			}
		
			Vertex vertices[4] = {
				{ {0,0,0,1},		  {0,0,0,1}, {0,0} },
				{ {size.x,0,0,1},     {0,0,0,1}, {1,0} },
				{ {size.y,size.y,0,1}, pallete,	 {1,1} },
				{ {0,size.y,0,1},	  {1,1,1,1}, {0,1} }
			};	
			InGuiRenderer::SubmitUI(position, vertices, 4, g_InContext->InGuiRenderData.ColorPickerTextureID);
			
			Separator();
			ColorPallete4("test", { size.x, 25.0f }, color);
			Separator();

			if (Slider("R: " + std::to_string(color.x), { 255,15 }, color.x, 255.0f))
				modified = true;
			Separator();
			if (Slider("G: " + std::to_string(color.y), { 255,15 }, color.y, 255.0f))
				modified = true;
			Separator();
			if (Slider("B: " + std::to_string(color.z), { 255,15 }, color.z, 255.0f))
				modified = true;
			Separator();

			return modified;
		}

		bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			glm::vec2 position = HandleWindowSpacing(size);

			Vertex vertices[20];
			Generate5SegmentColorRectangle(size, vertices);
		
			InGuiRenderer::SubmitUI(position, vertices, 20, g_InContext->InGuiRenderData.ColorPickerTextureID);
			return false;
		}

		bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
		{
			if (!g_InContext)
				return false;

			glm::vec4 color = g_InContext->InGuiConfig.DefaultColor;
			glm::vec4 panelColor = g_InContext->InGuiConfig.DefaultColor;
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
				panelColor = g_InContext->InGuiConfig.HooverColor;
			}

			glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y };
			glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x - InGuiWindow::PanelSize, panelPos.y };

			//HandleResize(it->second);
			InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,panelSize }, g_InContext->InGuiRenderData.SliderSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->InGuiRenderData.Font, name, panelPos, g_InContext->InGuiConfig.NameScale, it->second.Size.x);
			InGuiRenderer::SubmitUI({ 5,(panelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->InGuiRenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, g_InContext->InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->InGuiRenderData.TextureID, g_InContext->InGuiConfig.DefaultColor);

			if (!(it->second.Flags & Collapsed))
				InGuiRenderer::SubmitUI(rendererID, it->second.Position, it->second.Size, { 0,0,1,1 }, color);

			return (it->second.Flags & Hoovered);
		}

		void Separator()
		{
			g_InContext->InGuiData.WindowSpaceOffset.x = g_InContext->InGuiData.CurrentWindow->Size.x;
		}

	}
}