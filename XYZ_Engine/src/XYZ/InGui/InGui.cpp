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

			XYZ_ASSERT(!g_InContext->FrameData.CurrentWindow, "Missing end call");
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
			

			g_InContext->FrameData.MaxHeightInRow = 0.0f;
			g_InContext->FrameData.WindowSpaceOffset = { 0, currentWindow->Size.y };


			glm::vec4 panelColor = g_InContext->ConfigData.DefaultColor;
			
			if (currentWindow->Flags & Moved)
			{
				panelColor = g_InContext->ConfigData.HooverColor;
			}		
			if (currentWindow->Flags & Hoovered)
			{
				glm::vec4 color = g_InContext->ConfigData.DefaultColor;
				glm::vec2 winPos = currentWindow->Position;
				glm::vec2 winSize = currentWindow->Size;
				glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
				glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

				size_t lastFrameSize = currentWindow->Mesh.Vertices.size();
				currentWindow->Flags &= ~Modified;
				currentWindow->Mesh.TexturePairs.clear();
				currentWindow->Mesh.Vertices.clear();
				currentWindow->Mesh.Vertices.reserve(lastFrameSize);

				GenerateInGuiQuad(currentWindow->Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, panelColor);
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, name, panelPos, g_InContext->ConfigData.NameScale, currentWindow->Size.x, g_InContext->RenderData.FontTextureID);
				currentWindow->MinimalWidth = width + InGuiWindow::PanelSize;
				MoveVertices(currentWindow->Mesh, { 5, height / 2 }, 4, name.size() * 4);

				GenerateInGuiQuad(currentWindow->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, g_InContext->RenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID);
				if (!(currentWindow->Flags & Collapsed))
					GenerateInGuiQuad(currentWindow->Mesh, winPos, winSize, g_InContext->RenderData.WindowSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			}

			return !(currentWindow->Flags & Collapsed);
		}

		void End()
		{		
			InGuiRenderer::SubmitUI(g_InContext->FrameData.CurrentWindow->Mesh);
			g_InContext->FrameData.CurrentWindow = nullptr;
		}


		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			bool pressed = false;
			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size);


			if (currentWindow->Flags & Hoovered)
			{
				if (Collide(position, size, g_InContext->FrameData.MousePosition))
				{
					color = g_InContext->ConfigData.HooverColor;
					if (!(g_InContext->FrameData.Flags & ClickHandled))
					{
						pressed = g_InContext->FrameData.Flags & LeftMouseButtonDown;
						if (pressed)
							g_InContext->FrameData.Flags |= ClickHandled;
					}

				}
				GenerateInGuiQuad(currentWindow->Mesh, position, size, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
				size_t offset = currentWindow->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, size.x, g_InContext->RenderData.FontTextureID);
				glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2) - (height / 2) };
				MoveVertices(currentWindow->Mesh, position + textOffset, offset, name.size() * 4);
			}
			
			return pressed;
		}

		bool Checkbox(const std::string& name, const glm::vec2& size, bool& value)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
				
			if (currentWindow->Flags & Hoovered)
			{
				size_t offset = currentWindow->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, 1000.0f, g_InContext->RenderData.FontTextureID);
				glm::vec2 position = HandleWindowSpacing({ size.x + width + 5,size.y });
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
				MoveVertices(currentWindow->Mesh, position + textOffset, offset, name.size() * 4);

				if (Collide(position, size, g_InContext->FrameData.MousePosition))
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
					GenerateInGuiQuad(currentWindow->Mesh, position, size, g_InContext->RenderData.CheckboxSubTextureChecked->GetTexCoords(), g_InContext->RenderData.TextureID, color);
				else
					GenerateInGuiQuad(currentWindow->Mesh, position, size, g_InContext->RenderData.CheckboxSubTextureUnChecked->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			}
			
			
		
			return value;
		}

		bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			glm::vec2 handleSize = { size.y, size.y * 2 };

			bool modified = false;
			if (currentWindow->Flags & Hoovered)
			{
				size_t offset = currentWindow->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, name, {}, g_InContext->ConfigData.NameScale, size.x, g_InContext->RenderData.FontTextureID);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + width + 5, handleSize.y });
				glm::vec2 handlePos = { position.x + (value * valueScale) - handleSize.x / 2, position.y - (handleSize.x / 2) };
				MoveVertices(currentWindow->Mesh, position + textOffset, offset, name.size() * 4);

				if (Collide(position, size, g_InContext->FrameData.MousePosition))
				{
					color = g_InContext->ConfigData.HooverColor;
					modified = g_InContext->FrameData.Flags & LeftMouseButtonDown;
					if (modified && !(g_InContext->FrameData.Flags & ClickHandled))
					{
						float start = position.x;
						value = (g_InContext->FrameData.MousePosition.x - start) / valueScale;
					}
				}

				GenerateInGuiQuad(currentWindow->Mesh, position, size, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
				GenerateInGuiQuad(currentWindow->Mesh, handlePos, handleSize, g_InContext->RenderData.SliderHandleSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, color);
			}

			return modified;
		}

		bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;
			
			if (currentWindow->Flags & Hoovered)
			{
				glm::vec4 color = g_InContext->ConfigData.DefaultColor;
				glm::vec2 position = HandleWindowSpacing(size);

				if (Collide(position, size, g_InContext->FrameData.MousePosition))
				{
					color = g_InContext->ConfigData.HooverColor;
				}
				GenerateInGuiImage(currentWindow->Mesh, rendererID, position, size, { 0,0,1,1 }, color);
			}	
			return false;
		}

		bool Text(const std::string& text, const glm::vec2& scale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;


			if (currentWindow->Flags & Hoovered)
			{
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, text, {}, scale, g_InContext->ConfigData.MaxTextLength, g_InContext->RenderData.FontTextureID);
				glm::vec2 size = { width, height };
				glm::vec2 position = HandleWindowSpacing(size);
				glm::vec4 color = g_InContext->ConfigData.DefaultColor;

				if (Collide(position, size, g_InContext->FrameData.MousePosition))
				{
					color = g_InContext->ConfigData.HooverColor;
				}
			}
			return false;
		}

		bool ColorPicker4(const std::string& name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			if (currentWindow->Flags & Hoovered)
			{
				glm::vec2 position = HandleWindowSpacing(size);

				bool modified = false;
				if (Collide(position, size, g_InContext->FrameData.MousePosition))
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

				InGuiVertex vertices[4] = {
					{{0,0,0,1}, {position.x,         position.y,0},		     {0,0},g_InContext->RenderData.ColorPickerTextureID },
					{{0,0,0,1}, {position.x + size.x,position.y,0},          {1,0},g_InContext->RenderData.ColorPickerTextureID },
					{ pallete,  {position.x + size.x,position.y + size.y,0}, {1,1},g_InContext->RenderData.ColorPickerTextureID },
					{{1,1,1,1}, {position.x,         position.y + size.y,0}, {0,1},g_InContext->RenderData.ColorPickerTextureID }
				};
				GenerateInGuiQuad(currentWindow->Mesh, vertices, 4);
			
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
			return false;
		}

		bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			if (currentWindow->Flags & Hoovered)
			{
				glm::vec2 position = HandleWindowSpacing(size);

				Generate6SegmentColorRectangle(currentWindow->Mesh,position, size, g_InContext->RenderData.ColorPickerTextureID);
				bool modified = false;

				if (Collide(position, size, g_InContext->FrameData.MousePosition))
				{
					if (g_InContext->FrameData.Flags & LeftMouseButtonDown)
					{
						modified = true;
						color = ColorFrom6SegmentColorRectangle(position, size);
					}
				}
				return modified;
			}
			return false;
		}

		bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
		{
			if (!g_InContext)
				return false;
			XYZ_ASSERT(!g_InContext->FrameData.CurrentWindow, "Missing end call");

			glm::vec4 color = g_InContext->ConfigData.DefaultColor;
			glm::vec4 panelColor = g_InContext->ConfigData.DefaultColor;
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
			
			if (currentWindow->Flags & Moved)
			{
				panelColor = g_InContext->ConfigData.HooverColor;
			}
			// TODO SHOULD BE Modified FLAG NOT HOOVERED
			if (currentWindow->Flags & Hoovered)
			{
				glm::vec2 winPos = currentWindow->Position;
				glm::vec2 winSize = currentWindow->Size;
				glm::vec2 panelPos = { currentWindow->Position.x, currentWindow->Position.y + currentWindow->Size.y };
				glm::vec2 minButtonPos = { panelPos.x + currentWindow->Size.x - InGuiWindow::PanelSize, panelPos.y };

				size_t lastFrameSize = currentWindow->Mesh.Vertices.size();
				currentWindow->Flags &= ~Modified;
				currentWindow->Mesh.TexturePairs.clear();
				currentWindow->Mesh.Vertices.clear();
				currentWindow->Mesh.Vertices.reserve(lastFrameSize);

				GenerateInGuiQuad(currentWindow->Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, g_InContext->RenderData.SliderSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, panelColor);
				auto [width, height] = GenerateInGuiText(currentWindow->Mesh, g_InContext->RenderData.Font, name, panelPos, g_InContext->ConfigData.NameScale, currentWindow->Size.x, g_InContext->RenderData.FontTextureID);
				currentWindow->MinimalWidth = width + InGuiWindow::PanelSize;
				MoveVertices(currentWindow->Mesh, { 5, height / 2 }, 4, name.size() * 4);

				GenerateInGuiQuad(currentWindow->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, g_InContext->RenderData.MinimizeButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID);
				if (!(currentWindow->Flags & Collapsed))
					GenerateInGuiImage(currentWindow->Mesh, rendererID, winPos, winSize, { 0,0,1,1 });
			}


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

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* currentWindow = g_InContext->FrameData.CurrentWindow;

			if (currentWindow->Flags & Hoovered)
			{		
				if (g_InContext->FrameData.Flags & LeftMouseButtonDown)
				{
					glm::vec2 size = g_InContext->FrameData.MousePosition - g_InContext->FrameData.SelectedPoint;
					GenerateInGuiQuad(currentWindow->Mesh, g_InContext->FrameData.SelectedPoint, size, g_InContext->RenderData.ButtonSubTexture->GetTexCoords(), g_InContext->RenderData.TextureID, g_InContext->ConfigData.SelectColor);
					return glm::vec4(g_InContext->FrameData.SelectedPoint, g_InContext->FrameData.SelectedPoint + size);
				}
			}
			return glm::vec4(0);
		}

	}
}