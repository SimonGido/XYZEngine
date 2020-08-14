#include "stdafx.h"
#include "InGui.h"


#include "XYZ/Core/Application.h"
#include "XYZ/Core/WindowCodes.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/InGuiRenderer.h"

#include "InGuiHelper.h"

#include <ini.h>

namespace XYZ {

	namespace InGui {
	
		bool Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(!g_InContext->FrameData.CurrentWindow, "Missing end call");
			std::string copyName = name;
			std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = g_InContext->GetWindow(copyName);
			if (window == nullptr)
			{
				window = g_InContext->CreateWindow(copyName, position, size);
			}
			
			HandleActivity(window);

			frameData.MaxHeightInRow = 0.0f;
			frameData.WindowSpaceOffset = { 0, window->Size.y };
			frameData.MenuBarOffset = { 0,0 };

			glm::vec4 panelColor = configData.DefaultColor;
			
			if (window->Flags & Moved)
			{
				panelColor = configData.HooverColor;
			}		
			if (window->Flags & Modified)
			{
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 winPos = window->Position;
				glm::vec2 winSize = window->Size;
				glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
				glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

				size_t lastFrameSize = window->Mesh.Vertices.size();
				
				window->Mesh.TexturePairs.clear();
				window->Mesh.Vertices.clear();
				window->Mesh.Vertices.reserve(lastFrameSize);

				GenerateInGuiQuad(window->Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderData.SliderSubTexture->GetTexCoords(), renderData.TextureID, panelColor);
				if (!(window->Flags & MenuEnabled))
				{
					auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, name, panelPos, configData.NameScale, window->Size.x, renderData.FontTextureID);
					window->MinimalWidth = width + InGuiWindow::PanelSize;
					MoveVertices(window->Mesh.Vertices.data(), { 5, height / 2 }, 4, name.size() * 4);
				}
				GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderData.MinimizeButtonSubTexture->GetTexCoords(), renderData.TextureID);
				if (!(window->Flags & Collapsed))
					GenerateInGuiQuad(window->Mesh, winPos, winSize, g_InContext->RenderData.WindowSubTexture->GetTexCoords(), renderData.TextureID, color);
			}

			return !(window->Flags & Collapsed);
		}

		void End()
		{		
			XYZ_ASSERT(!(g_InContext->FrameData.CurrentWindow->Flags & MenuEnabled), "Missing menu end call");
			g_InContext->FrameData.CurrentWindow->Flags &= ~Modified;
			g_InContext->FrameData.CurrentWindow = nullptr;
		}


		bool MenuBar(const std::string& name, bool& open)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;
			
			InGuiWindow* window = frameData.CurrentWindow;
			window->Flags |= MenuEnabled;
			if (window->Flags & Modified)
			{
				float xOffset = 10.0f;
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 pos = { window->Position.x + frameData.MenuBarOffset.x, window->Position.y + window->Size.y };
		
				std::vector<InGuiVertex> textVertices(name.size() * 4);
				auto [width, height] = GenerateInGuiText(textVertices.data(), renderData.Font, name, pos, configData.NameScale, window->Size.x,renderData.FontTextureID);
				MoveVertices(textVertices.data(), { xOffset, height / 2 }, 0, name.size() * 4);
				width += xOffset * 2;

				if (Collide(pos, { width,InGuiWindow::PanelSize }, frameData.MousePosition))
				{
					color = configData.HooverColor;
					if ((frameData.Flags & LeftMouseButtonDown)
						&& !(frameData.Flags & ClickHandled))
					{
						open = !open;
					}
				}
				if (window->Flags & Moved)
				{
					color = configData.HooverColor;
				}
					
				GenerateInGuiQuad(window->Mesh, pos, { width, InGuiWindow::PanelSize }, renderData.SliderSubTexture->GetTexCoords(), renderData.TextureID, color);
			
				for (auto& vertex : textVertices)
					window->Mesh.Vertices.push_back(vertex);

				
				frameData.LastMenuBarWidth = width;
			}

			return open;
		}

		bool MenuItem(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;
			
			if (window->Flags & Modified)
			{
				g_InContext->FrameData.MenuBarOffset.y += size.y;
				bool pressed = false;
				float xOffset = 10.0f;
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 pos = { window->Position.x + frameData.MenuBarOffset.x, window->Position.y + window->Size.y - frameData.MenuBarOffset.y };

				std::vector<InGuiVertex> textVertices(name.size() * 4);
				auto [width, height] = GenerateInGuiText(textVertices.data(), renderData.Font, name, pos, configData.NameScale, window->Size.x, renderData.FontTextureID);
				MoveVertices(textVertices.data(), { xOffset, height / 2 }, 0, name.size() * 4);
				width += xOffset * 2;

				if (Collide(pos, { width,InGuiWindow::PanelSize }, frameData.MousePosition))
				{
					color = configData.HooverColor;
					if (!(frameData.Flags & ClickHandled))
					{
						pressed = frameData.Flags & LeftMouseButtonDown;
					}
				}

				GenerateInGuiQuad(window->Mesh, pos, size, renderData.SliderSubTexture->GetTexCoords(),renderData.TextureID, color);
				for (auto& vertex : textVertices)
					window->Mesh.Vertices.push_back(vertex);

				

				return pressed;
			}		
			return false;
		}

		void MenuEnd()
		{
			if (!g_InContext)
				return;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiWindow* window = g_InContext->FrameData.CurrentWindow;

			window->Flags &= ~MenuEnabled;

			g_InContext->FrameData.MenuBarOffset.x += g_InContext->FrameData.LastMenuBarWidth;
		}

		bool Button(const std::string& name, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;

			bool pressed = false;
		
			if (window->Flags & Modified)
			{
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 position = HandleWindowSpacing(size);

				if (Collide(position, size, frameData.MousePosition))
				{
					color = configData.HooverColor;
					if (!(frameData.Flags & ClickHandled))
					{
						pressed = frameData.Flags & LeftMouseButtonDown;
						if (pressed)
							frameData.Flags |= ClickHandled;
					}

				}
				GenerateInGuiQuad(window->Mesh, position, size, renderData.ButtonSubTexture->GetTexCoords(), renderData.TextureID, color);
				size_t offset = window->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, name, {}, configData.NameScale, size.x, renderData.FontTextureID);
				glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2) - (height / 2) };
				MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
			}
			
			return pressed;
		}

		bool Checkbox(const std::string& name, const glm::vec2& size, bool& value)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;
		
			if (window->Flags & Modified)
			{
				glm::vec4 color = configData.DefaultColor;
				size_t offset = window->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, name, {}, configData.NameScale, 1000.0f, renderData.FontTextureID);
				glm::vec2 position = HandleWindowSpacing({ size.x + width + 5,size.y });
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
				MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);

				if (Collide(position, size, frameData.MousePosition))
				{
					color = configData.HooverColor;
					if ((frameData.Flags & LeftMouseButtonDown)
						&& !(frameData.Flags & ClickHandled))
					{
						value = !value;
						frameData.Flags |= ClickHandled;
					}
				}

				if (value)
					GenerateInGuiQuad(window->Mesh, position, size, renderData.CheckboxSubTextureChecked->GetTexCoords(), renderData.TextureID, color);
				else
					GenerateInGuiQuad(window->Mesh, position, size, renderData.CheckboxSubTextureUnChecked->GetTexCoords(), renderData.TextureID, color);
			}
			
			
		
			return value;
		}

		bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;


			bool modified = false;
			if (window->Flags & Modified)
			{		
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 handleSize = { size.y, size.y * 2 };

				size_t offset = window->Mesh.Vertices.size();
				auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, name, {}, configData.NameScale, size.x, renderData.FontTextureID);
				glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
				glm::vec2 position = HandleWindowSpacing({ size.x + width + 5, handleSize.y });
				glm::vec2 handlePos = { position.x + (value * valueScale) - handleSize.x / 2, position.y - (handleSize.x / 2) };
				MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);

				if (Collide(position,size,frameData.MousePosition))
				{
					color = configData.HooverColor;
					modified = frameData.Flags & LeftMouseButtonDown;
					if (modified)
					{		
						float start = position.x;
						value = (frameData.MousePosition.x - start) / valueScale;
					}
				}
				GenerateInGuiQuad(window->Mesh, position, size, renderData.SliderSubTexture->GetTexCoords(), renderData.TextureID, color);
				GenerateInGuiQuad(window->Mesh, handlePos, handleSize, renderData.SliderHandleSubTexture->GetTexCoords(), renderData.TextureID, color);
			}

			return modified;
		}

		bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;
			
			if (window->Flags & Modified)
			{
				glm::vec4 color = configData.DefaultColor;
				glm::vec2 position = HandleWindowSpacing(size);

				if (Collide(position, size, frameData.MousePosition))
				{
					color = configData.HooverColor;
				}
				GenerateInGuiImage(window->Mesh, rendererID, position, size, { 0,0,1,1 }, color);
			}	
			return false;
		}

		bool Text(const std::string& text, const glm::vec2& scale)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;

			if (window->Flags & Modified)
			{
				auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, text, {}, scale, configData.MaxTextLength, g_InContext->RenderData.FontTextureID);
				glm::vec2 size = { width, height };
				glm::vec2 position = HandleWindowSpacing(size);
				glm::vec4 color = configData.DefaultColor;

				if (Collide(position, size, frameData.MousePosition))
				{
					color = configData.HooverColor;
				}
			}
			return false;
		}

		bool ColorPicker4(const std::string& name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;

			InGuiWindow* window = frameData.CurrentWindow;

			if (window->Flags & Modified)
			{
				glm::vec2 position = HandleWindowSpacing(size);

				bool modified = false;
				
				Separator();
				if (ColorPallete4("test", { size.x, 25.0f }, pallete))
				{
					modified = true;
				}
				Separator();

				InGuiVertex vertices[4] = {
					{{0,0,0,1}, {position.x,         position.y,0},		     {0,0},renderData.ColorPickerTextureID },
					{{0,0,0,1}, {position.x + size.x,position.y,0},          {1,0},renderData.ColorPickerTextureID },
					{ pallete,  {position.x + size.x,position.y + size.y,0}, {1,1},renderData.ColorPickerTextureID },
					{{1,1,1,1}, {position.x,         position.y + size.y,0}, {0,1},renderData.ColorPickerTextureID }
				};
				GenerateInGuiQuad(window->Mesh, vertices, 4);
			
				if (Slider("R: " + std::to_string(color.x), { size.x,15 }, color.x, size.x))
					modified = true;
				Separator();
				if (Slider("G: " + std::to_string(color.y), { size.x,15 }, color.y, size.x))
					modified = true;
				Separator();
				if (Slider("B: " + std::to_string(color.z), { size.x,15 }, color.z, size.x))
					modified = true;
				Separator();


				if ((Collide(position, size, frameData.MousePosition) && frameData.Flags & LeftMouseButtonDown))
				{
					modified = true;
					color = CalculatePixelColor(pallete, position, size);
				}

				return modified;
			}
			return false;
		}

		bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color)
		{
			if (!g_InContext)
				return false;

			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;

			InGuiWindow* window = frameData.CurrentWindow;

			if (window->Flags & Modified)
			{
				glm::vec2 position = HandleWindowSpacing(size);

				Generate6SegmentColorRectangle(window->Mesh,position, size, renderData.ColorPickerTextureID);
				bool modified = false;

				if (Collide(position, size, frameData.MousePosition))
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

			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = g_InContext->GetWindow(copyName);
			if (window == nullptr)
			{
				window = g_InContext->CreateWindow(copyName, position, size);
			}		
			HandleActivity(window);
			
			if (window->Flags & Moved)
			{
				panelColor = configData.HooverColor;
			}

			if (window->Flags & Modified)
			{
				glm::vec2 winPos = window->Position;
				glm::vec2 winSize = window->Size;
				glm::vec2 panelPos = { window->Position.x, window->Position.y + window->Size.y };
				glm::vec2 minButtonPos = { panelPos.x + window->Size.x - InGuiWindow::PanelSize, panelPos.y };

				size_t lastFrameSize = window->Mesh.Vertices.size();
				window->Flags &= ~Modified;
				window->Mesh.TexturePairs.clear();
				window->Mesh.Vertices.clear();
				window->Mesh.Vertices.reserve(lastFrameSize);

				GenerateInGuiQuad(window->Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderData.SliderSubTexture->GetTexCoords(), renderData.TextureID, panelColor);
				auto [width, height] = GenerateInGuiText(window->Mesh, renderData.Font, name, panelPos, configData.NameScale, window->Size.x, renderData.FontTextureID);
				window->MinimalWidth = width + InGuiWindow::PanelSize;
				MoveVertices(window->Mesh.Vertices.data(), { 5, height / 2 }, 4, name.size() * 4);

				GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderData.MinimizeButtonSubTexture->GetTexCoords(), renderData.TextureID);
				if (!(window->Flags & Collapsed))
					GenerateInGuiImage(window->Mesh, rendererID, winPos, winSize, { 0,0,1,1 });
			}


			return (window->Flags & Hoovered);
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
			InGuiFrameData& frameData = g_InContext->FrameData;
			InGuiRenderData& renderData = g_InContext->RenderData;
			InGuiConfig& configData = g_InContext->ConfigData;

			InGuiWindow* window = frameData.CurrentWindow;

			if (window->Flags & Hoovered)
			{		
				if (frameData.Flags & LeftMouseButtonDown)
				{
					glm::vec2 size = frameData.MousePosition - frameData.SelectedPoint;
					GenerateInGuiQuad(window->Mesh, frameData.SelectedPoint, size, renderData.ButtonSubTexture->GetTexCoords(), renderData.TextureID, configData.SelectColor);
					return glm::vec4(frameData.SelectedPoint, frameData.SelectedPoint + size);
				}
			}
			return glm::vec4(0);
		}

	}
}