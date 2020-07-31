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

			static glm::vec4 panelColor = g_InContext->ConfigData.DefaultColor;
			glm::vec4 color = g_InContext->ConfigData.DefaultColor;

			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			InGuiWindow* currentWindow = g_InContext->GetWindow(copyName);
			if (currentWindow == nullptr)
			{
				currentWindow = g_InContext->CreateWindow(copyName, position, size);
			}

			
			HandleMouseInput(*currentWindow);		
			HandleResize(*currentWindow);
			HandleDocking(*currentWindow);
			HandleMove(*currentWindow);

			glm::vec2 winPos = currentWindow->Position;
			glm::vec2 winSize = currentWindow->Size;
			
	
			
		
			g_InContext->FrameData.MaxHeightInRow = 0.0f;
			g_InContext->FrameData.WindowSpaceOffset = { 0, winSize.y };

			glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
			glm::vec2 minButtonPos = { panelPos.x +winSize.x - InGuiWindow::PanelSize, panelPos.y };
			
			InGuiRenderer::SubmitUI(panelPos, { winSize.x ,InGuiWindow::PanelSize }, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->RenderData.Font, name, panelPos, g_InContext->ConfigData.NameScale, currentWindow->Size.x);
			InGuiRenderer::SubmitUI({ 5,(InGuiWindow::PanelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->RenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, g_InContext->RenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, { 1,1,1,1 });
			
			if (!(currentWindow->Flags & Collapsed))
				InGuiRenderer::SubmitUI(winPos, winSize, g_InContext->RenderData.WindowSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);

			if (currentWindow->Flags & Moved)
			{
				panelColor = g_InContext->ConfigData.HooverColor;
			}
			else
				panelColor = g_InContext->ConfigData.DefaultColor;

			return !(currentWindow->Flags & Collapsed);
		}

		void End()
		{
			g_InContext->FrameData.CurrentWindow = nullptr;
		}


		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			
			bool pressed = false;
			glm::vec4 color = g_InContext->ConfigData.DefaultColor;

			InGuiText text;
			GenerateInGuiText(text, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, size.x);
			glm::vec2 textOffset = { (size.x / 2) - (text.Width / 2),(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing(size);

			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				color = g_InContext->ConfigData.HooverColor;
				if (!(g_InContext->FrameData.Flags & ClickHandled))
				{
					pressed = g_InContext->FrameData.Flags & LeftMouseButtonDown;
					if (pressed)
						g_InContext->FrameData.Flags |= ClickHandled;
				}
			}

			InGuiRenderer::SubmitUI(position, size, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->RenderData.FontTextureID);

			return pressed;
		}

		bool Checkbox(const std::string& name, const glm::vec2& size, bool& value)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;

			InGuiText text;
			GenerateInGuiText(text, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, g_InContext->ConfigData.MaxTextLength);
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5,size.y });

			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				color = g_InContext->ConfigData.HooverColor;
				if ((g_InContext->FrameData.Flags & LeftMouseButtonDown) 
				&& !(g_InContext->FrameData.Flags & ClickHandled))
				{
					value = !value;
					g_InContext->FrameData.Flags |= ClickHandled;
				}
			}

			if (value)
				InGuiRenderer::SubmitUI(position, size, g_InContext->RenderData.CheckboxSubTextureChecked->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			else
				InGuiRenderer::SubmitUI(position, size, g_InContext->RenderData.CheckboxSubTextureUnChecked->GetTexCoords(), g_InContext->RenderData.TextureID, color);

			InGuiRenderer::SubmitUI(position + textOffset, text.Vertices.data(), text.Vertices.size(), g_InContext->RenderData.FontTextureID);
		
			return value;
		}

		bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
		
			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			glm::vec2 handleSize = { size.y, size.y * 2 };

			InGuiText text;
			GenerateInGuiText(text, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, size.x);
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (text.Height / 2) };
			glm::vec2 position = HandleWindowSpacing({ size.x + text.Width + 5, handleSize.y });

			glm::vec2 handlePos = { position.x + (value * valueScale) - handleSize.x/2, position.y - (handleSize.x / 2) };

			bool modified = false;
			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				color = g_InContext->ConfigData.HooverColor;
				modified = g_InContext->FrameData.Flags & LeftMouseButtonDown;
				if (modified && !(g_InContext->FrameData.Flags & ClickHandled))
				{
					float start = position.x;
					value = (g_InContext->FrameData.MousePosition.x - start) / valueScale;
				}
			}


			InGuiRenderer::SubmitUI(position, size, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			InGuiRenderer::SubmitUI(handlePos, handleSize, g_InContext->RenderData.SliderHandleSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);


			InGuiRenderer::SubmitUI(textOffset + position, text.Vertices.data(), text.Vertices.size(), g_InContext->RenderData.FontTextureID);

			return modified;
		}

		bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size);
			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				color = g_InContext->ConfigData.HooverColor;
			}
			InGuiRenderer::SubmitUI(rendererID, position, size, { 0,0,1,1 }, color);
			
			return false;
		}

		bool Text(const std::string& text, const glm::vec2& scale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			
			InGuiText txt;
			GenerateInGuiText(txt, g_InContext->RenderData.Font, text, {}, scale, g_InContext->ConfigData.MaxTextLength);
			glm::vec2 size = { txt.Width, txt.Height };
			glm::vec2 position = HandleWindowSpacing(size);
			glm::vec4 color = g_InContext->ConfigData.DefaultColor;

			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				color = g_InContext->ConfigData.HooverColor;
			}

			InGuiRenderer::SubmitUI(position, txt.Vertices.data(), txt.Vertices.size(), g_InContext->RenderData.FontTextureID);
			
			return false;
		}

		bool ColorPicker4(const std::string& name, const glm::vec2 & size, glm::vec4& pallete, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			glm::vec2 position = HandleWindowSpacing(size);
				
			bool modified = false;
			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{		
				modified = g_InContext->FrameData.Flags & LeftMouseButtonDown;
				if (modified)
				{
					color = CalculatePixelColor(pallete, position, size);
				}
			}

			Separator();
			if (ColorPallete4("test", { size.x, 25.0f }, pallete))
			{
				modified = true;
			}
			Separator();
		
			Vertex vertices[4] = {
				{ {0,0,0,1},		  {0,0,0,1}, {0,0} },
				{ {size.x,0,0,1},     {0,0,0,1}, {1,0} },
				{ {size.y,size.y,0,1}, pallete,	 {1,1} },
				{ {0,size.y,0,1},	  {1,1,1,1}, {0,1} }
			};	
			InGuiRenderer::SubmitUI(position, vertices, 4, g_InContext->RenderData.ColorPickerTextureID);
			
			
			if (Slider("R: " + std::to_string(color.x), { size.x,15 }, color.x, size.x))
				modified = true;
			Separator();
			if (Slider("G: " + std::to_string(color.y), { size.x,15 }, color.y, size.x))
				modified = true;
			Separator();
			if (Slider("B: " + std::to_string(color.z), { size.x,15 }, color.z, size.x))
				modified = true;
			Separator();

			return modified;
		}

		bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			static constexpr uint32_t numVertices = 24;

			glm::vec2 position = HandleWindowSpacing(size);

			Vertex vertices[numVertices];
			Generate6SegmentColorRectangle(size, vertices);
			bool modified = false;
			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				if (g_InContext->FrameData.Flags & LeftMouseButtonDown)
				{
					modified = true;
					color = ColorFrom6SegmentColorRectangle(position, size);
				}
			}

			InGuiRenderer::SubmitUI(position, vertices, numVertices, g_InContext->RenderData.ColorPickerTextureID);
			return modified;
		}

		bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
		{
			if (!g_InContext)
				return false;

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			static glm::vec4 panelColor = g_InContext->ConfigData.DefaultColor;
			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			InGuiWindow* currentWindow = g_InContext->GetWindow(copyName);
			if (currentWindow == nullptr)
			{
				currentWindow = g_InContext->CreateWindow(copyName, position, size);
			}
	
			HandleMouseInput(*currentWindow);
			HandleResize(*currentWindow);
			HandleDocking(*currentWindow);
			HandleMove(*currentWindow);	


			glm::vec2 panelPos = { currentWindow->Position.x, currentWindow->Position.y + currentWindow->Size.y };
			glm::vec2 minButtonPos = { panelPos.x + currentWindow->Size.x - InGuiWindow::PanelSize, panelPos.y };


			InGuiRenderer::SubmitUI(panelPos, { currentWindow->Size.x ,panelSize }, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, panelColor);
			InGuiText text;
			GenerateInGuiText(text, g_InContext->RenderData.Font, name, panelPos, g_InContext->ConfigData.NameScale, currentWindow->Size.x);
			InGuiRenderer::SubmitUI({ 5,(panelSize / 2) - (text.Height / 2) }, text.Vertices.data(), text.Vertices.size(), g_InContext->RenderData.FontTextureID);
			InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, g_InContext->RenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.DefaultColor);

			if (!(currentWindow->Flags & Collapsed))
				InGuiRenderer::SubmitUI(rendererID, currentWindow->Position, currentWindow->Size, { 0,0,1,1 }, color);

			if (currentWindow->Flags & Moved)
			{
				panelColor = g_InContext->ConfigData.HooverColor;
			}
			else
				panelColor = g_InContext->ConfigData.DefaultColor;

			return (currentWindow->Flags & Hoovered);
		}

		void Separator()
		{
			g_InContext->FrameData.WindowSpaceOffset.x = g_InContext->FrameData.CurrentWindow->Size.x;
		}

		glm::vec4 Selector()
		{
			if (!g_InContext)
				return glm::vec4(0);

			bool activeWindow = g_InContext->FrameData.CurrentWindow->Flags & Hoovered;
			if (activeWindow && g_InContext->FrameData.Flags & LeftMouseButtonDown)
			{
				glm::vec2 size = g_InContext->FrameData.MousePosition - g_InContext->FrameData.SelectedPoint;
				InGuiRenderer::SubmitUI(g_InContext->FrameData.SelectedPoint, size, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
			
				return glm::vec4(g_InContext->FrameData.SelectedPoint, g_InContext->FrameData.SelectedPoint + size);
			}

			return glm::vec4(0);
		}

	}
}