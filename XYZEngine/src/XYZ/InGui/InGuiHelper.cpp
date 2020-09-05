#include "stdafx.h"
#include "InGuiHelper.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/InGuiRenderer.h"

namespace XYZ {
	namespace InGui {
		extern InGuiContext* g_InContext;

		static enum Color
		{
			RedToGreen,
			GreenToBlue,
			BlueToRed,
			None
		};

		glm::vec2 MouseToWorld(const glm::vec2& point)
		{
			glm::vec2 offset = { g_InContext->FrameData.WindowSize.x / 2,g_InContext->FrameData.WindowSize.y / 2 };
			return { point.x - offset.x, offset.y - point.y };
		}
		glm::vec2 HandleWindowSpacing(const glm::vec2& uiSize)
		{
			XYZ_ASSERT(g_InContext->FrameData.CurrentWindow, "Missing begin call");

			// Set position to the position of current window
			glm::vec2 position = g_InContext->FrameData.CurrentWindow->Position;
			glm::vec2 offset = { 10, 10 };

			// Find the highest widget in row
			if (uiSize.y > g_InContext->FrameData.MaxHeightInRow)
				g_InContext->FrameData.MaxHeightInRow = uiSize.y;


			// If widget position is going to be outside of the window
			if (g_InContext->FrameData.CurrentWindow->Size.x <= g_InContext->FrameData.WindowSpaceOffset.x + uiSize.x)
			{
				// Set window offset x to zero
				g_InContext->FrameData.WindowSpaceOffset.x = 0.0f;

				// Subtract the highest widget in row and offset y from window offset y 
				g_InContext->FrameData.WindowSpaceOffset.y -= g_InContext->FrameData.MaxHeightInRow + offset.y;

				g_InContext->FrameData.MaxHeightInRow = uiSize.y;
			}

			// Subtract from position widget size y
			position.y -= uiSize.y;
			// Add to position window space offset and offset
			position += g_InContext->FrameData.WindowSpaceOffset + glm::vec2{ offset.x,-offset.y };


			g_InContext->FrameData.WindowSpaceOffset.x += uiSize.x + offset.x;

			return position;
		}
		glm::vec4 CalculatePixelColor(const glm::vec4& pallete, const glm::vec2& position, const glm::vec2& size)
		{
			glm::vec2 pos = g_InContext->FrameData.MousePosition - position;
			glm::vec2 scale = pos / size;

			glm::vec4 diff = pallete - glm::vec4{1, 1, 1, 1};

			glm::vec4 result = { scale.y,scale.y, scale.y,1 };
			result += glm::vec4(scale.y * scale.x * diff.x, scale.y * scale.x * diff.y, scale.y * scale.x * diff.z, 0);
			
			return result;
		}

		glm::vec4 ColorFrom6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size)
		{
			static constexpr uint32_t numColorSegments = 3;
			
			float segmentSize = size.x / numColorSegments;
			float pos = g_InContext->FrameData.MousePosition.x - position.x;
			uint32_t segment = (uint32_t)floor(pos / segmentSize);
			float distance = (pos - (segment * segmentSize)) / segmentSize;
			

			switch (segment)
			{
			case RedToGreen:
				return { 1.0f - distance, distance,0.0f,1.0f };
			case GreenToBlue:
				return { 0.0f, 1.0f - distance, distance, 1.0f };
			case BlueToRed:
				return { distance,0.0f,1.0f - distance,1.0f };
			case None:
				return { distance,0.0f,1.0f - distance,1.0f };
			}
			
			return { 0,1,0,1 };
		}

		bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
					pos.x		   < point.x &&
					pos.y + size.y >  point.y &&
					pos.y		   < point.y);
		}

		bool DetectMove(const InGuiWindow& window)
		{
			return (g_InContext->FrameData.MousePosition.y >= window.Position.y + window.Size.y
				&& !(window.Flags & Moved));
		}

		bool CollideSquares(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& otherPos, const glm::vec2& otherSize)
		{
			if (pos.x >= otherPos.x + otherSize.x || otherPos.x >= pos.x + size.x)
				return false;

			if (pos.y + size.y <= otherPos.y || otherPos.y + otherSize.y <= pos.y)
				return false;


			return true;
		}

		bool DetectResize(const InGuiWindow& window)
		{
			if (!(window.Flags & Docked) && !(window.Flags & Moved))
			{
				auto& app = Application::Get();
				glm::vec2 offset = { 10,10 };
				auto& mousePos = g_InContext->FrameData.MousePosition;

				// Right side
				if (mousePos.x >= window.Position.x + window.Size.x - offset.x)
				{
					g_InContext->FrameData.Flags |= WindowRightResize;
					app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
				}
				// Left side
				else if (mousePos.x <= window.Position.x + offset.x)
				{
					g_InContext->FrameData.Flags |= WindowLeftResize;
					app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);

				}
				// Bottom side
				if (mousePos.y <= window.Position.y + offset.y)
				{
					g_InContext->FrameData.Flags |= WindowBottomResize;
					app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);

				}
				// Top side
				else if (mousePos.y >= window.Position.y + window.Size.y - offset.y + InGuiWindow::PanelSize)
				{
					g_InContext->FrameData.Flags |= WindowTopResize;
					app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);

				}
				if (g_InContext->FrameData.Flags
					& (WindowRightResize | WindowLeftResize | WindowBottomResize | WindowTopResize))
				{
					return true;
				}
			}
			return false;
		}


		bool DetectCollapse(const InGuiWindow& window)
		{
			glm::vec2 minButtonPos = { 
				window.Position.x + window.Size.x - InGuiWindow::PanelSize, 
				window.Position.y + window.Size.y 
			};
			glm::vec2 minButtonSize = { InGuiWindow::PanelSize,InGuiWindow::PanelSize };

			if (Collide(minButtonPos, minButtonSize, g_InContext->FrameData.MousePosition))
			{
				return true;
			}
			return false;
		}


	
		void HandleMouseInput(InGuiWindow* window)
		{
			if (window && !(g_InContext->FrameData.Flags & ClickHandled))
			{
				glm::vec2 size = { window->Size.x,window->Size.y + InGuiWindow::PanelSize };
				glm::vec2 position = window->Position;
				if (window->Flags & Collapsed)
				{
					size.y -= window->Size.y;
					position.y += window->Size.y;
				}

				if (g_InContext->FrameData.Flags & RightMouseButtonDown)
				{
					if (DetectResize(*window))
					{
						window->Flags |= Resized;
						g_InContext->FrameData.Flags |= ClickHandled;
					}
					else if (DetectMove(*window))
					{
						window->Flags |= Moved;
						g_InContext->FrameData.Flags |= ClickHandled;
						g_InContext->FrameData.ModifiedWindowMouseOffset = g_InContext->FrameData.MousePosition - window->Position - glm::vec2{ 0, window->Size.y };
					}
				}
				else if (g_InContext->FrameData.Flags & LeftMouseButtonDown)
				{
					if (DetectCollapse(*window))
					{
						window->Flags ^= Collapsed;
						g_InContext->FrameData.Flags |= ClickHandled;
					}
				}
			}
		}

		void HandleResize(InGuiWindow* window)
		{		
			auto& mousePos = g_InContext->FrameData.MousePosition;
			if (window && (window->Flags & Resized) && !(window->Flags & Docked))
			{
				window->Flags |= Modified;
				if (window->Size.x > window->MinimalWidth)
				{
					if (g_InContext->FrameData.Flags & WindowRightResize)
					{
						window->Size.x = mousePos.x - window->Position.x;
					}
					else if (g_InContext->FrameData.Flags & WindowLeftResize)
					{
						window->Size.x = window->Position.x + window->Size.x - mousePos.x;
						window->Position.x = mousePos.x;
					}
				}
				else
				{
					window->Size.x += 5;
					g_InContext->FrameData.Flags &= ~WindowLeftResize;
					g_InContext->FrameData.Flags &= ~WindowRightResize;
				}


				if (g_InContext->FrameData.Flags & WindowBottomResize)
				{
					window->Size.y = window->Position.y + window->Size.y - mousePos.y;
					window->Position.y = mousePos.y;
				}
				else if (g_InContext->FrameData.Flags & WindowTopResize)
				{
					window->Size.y = mousePos.y - window->Position.y - InGuiWindow::PanelSize;
				}

			
				if (!(g_InContext->FrameData.Flags & RightMouseButtonDown))
				{
					auto& app = Application::Get();
					app.GetWindow().SetCursor(XYZ_ARROW_CURSOR);
					window->Flags &= ~(Resized);
				}
			}
		}

		void HandleMove(InGuiWindow* window)
		{
			if (window && (window->Flags & Moved))
			{
				glm::vec2 pos = g_InContext->FrameData.MousePosition - g_InContext->FrameData.ModifiedWindowMouseOffset;
				window->Position = { pos.x, pos.y - window->Size.y };
				g_InContext->DockSpace->RemoveWindow(window);
				window->Flags |= Modified;
				
				
				if (!(g_InContext->FrameData.Flags & RightMouseButtonDown))
				{
					window->Flags &= ~(Docked);			
					if (g_InContext->FrameData.Flags & DockingHandled)
					{
						HandleDocking(window);
					}
					window->Flags &= ~(Moved);
				}	
			}
		}

		void HandleDocking(InGuiWindow* window)
		{
			if (window)
			{
				if ((g_InContext->FrameData.Flags & DockingHandled)
					&& (window->Flags & Moved)
					&& !(window->Flags & Docked))
				{
					g_InContext->DockSpace->InsertWindow(window, g_InContext->FrameData.MousePosition);
					g_InContext->FrameData.Flags &= ~DockingHandled;

					if (window->DockNode)
					{
						window->Size = { window->DockNode->Size.x, window->DockNode->Size.y - InGuiWindow::PanelSize };
						window->Position = window->DockNode->Position;
					}
				}
			}
		}

		void HandleWindowActivity(InGuiWindow* window)
		{
			glm::vec2 size = { window->Size.x,window->Size.y + InGuiWindow::PanelSize };
			glm::vec2 position = window->Position;
			if (window->Flags & Collapsed)
			{
				size.y -= window->Size.y;
				position.y += window->Size.y;
			}

			if (Collide(position, size, g_InContext->FrameData.MousePosition))
			{
				window->Flags |= Hoovered;
				window->Flags |= Modified;

				if (g_InContext->FrameData.Flags & LeftMouseButtonDown
				 || g_InContext->FrameData.Flags & RightMouseButtonDown)
				{
					g_InContext->FrameData.LastActiveWindow = window;
				}
			}
			else
			{
				window->Flags &= ~Hoovered;
			}
		}
	
		void Generate6SegmentColorRectangle(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, uint32_t textureID)
		{
			static constexpr uint32_t numSegments = 6;
			static constexpr uint32_t numVertices = numSegments * 4;
			float segmentSize = size.x / numSegments;

			size_t vertexOffset = mesh.Vertices.size();
			uint32_t counter = 0;
			
			float offset = 0.0f;
			while (counter < numVertices)
			{
				mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset, position.y, 0.0f },                 { 0,0 }, textureID });
				mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize, position.y, 0.0f },   { 1,0 }, textureID });
				mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize,position.y + size.y, 0.0f }, { 1,1 }, textureID });
				mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset,position.y + size.y, 0.0f },               { 0,1 }, textureID });
				
				offset += segmentSize;
				counter += 4;
			}

			mesh.Vertices[vertexOffset + 0].Color = { 1,0,0,1 };
			mesh.Vertices[vertexOffset + 1].Color = { 1,1,0,1 };
			mesh.Vertices[vertexOffset + 2].Color = { 1,1,0,1 };
			mesh.Vertices[vertexOffset + 3].Color = { 1,0,0,1 };
			//////////////////////////////
			mesh.Vertices[vertexOffset + 4].Color = { 1,1,0,1 };
			mesh.Vertices[vertexOffset + 5].Color = { 0,1,0,1 };
			mesh.Vertices[vertexOffset + 6].Color = { 0,1,0,1 };
			mesh.Vertices[vertexOffset + 7].Color = { 1,1,0,1 };
			//////////////////////////////
			mesh.Vertices[vertexOffset + 8].Color =  { 0,1,0,1 };
			mesh.Vertices[vertexOffset + 9].Color =  { 0,1,1,1 };
			mesh.Vertices[vertexOffset + 10].Color = { 0,1,1,1 };
			mesh.Vertices[vertexOffset + 11].Color = { 0,1,0,1 };
			//////////////////////////////
			mesh.Vertices[vertexOffset + 12].Color = { 0,1,1,1 };
			mesh.Vertices[vertexOffset + 13].Color = { 0,0,1,1 };
			mesh.Vertices[vertexOffset + 14].Color = { 0,0,1,1 };
			mesh.Vertices[vertexOffset + 15].Color = { 0,1,1,1 };
			//////////////////////////////
			mesh.Vertices[vertexOffset + 16].Color = { 0,0,1,1 };
			mesh.Vertices[vertexOffset + 17].Color = { 1,0,1,1 };
			mesh.Vertices[vertexOffset + 18].Color = { 1,0,1,1 };
			mesh.Vertices[vertexOffset + 19].Color = { 0,0,1,1 };
			//////////////////////////////
			mesh.Vertices[vertexOffset + 20].Color = { 1,0,1,1 };
			mesh.Vertices[vertexOffset + 21].Color = { 1,0,0,1 };
			mesh.Vertices[vertexOffset + 22].Color = { 1,0,0,1 };
			mesh.Vertices[vertexOffset + 23].Color = { 1,0,1,1 };
		}
		void GenerateInGuiQuad(InGuiMesh& mesh, InGuiVertex* vertices, size_t count)
		{
			for (size_t i = 0; i < count; ++i)
				mesh.Vertices.push_back(vertices[i]);
		}
		void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
		{
			constexpr size_t quadVertexCount = 4;
			glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
			};

			glm::vec4 quadVertexPositions[4] = {
				{ position.x ,		  position.y , 0.0f, 1.0f },
				{ position.x + size.x,position.y , 0.0f, 1.0f },
				{ position.x + size.x,position.y + size.y, 0.0f, 1.0f },
				{ position.x ,        position.y + size.y, 0.0f, 1.0f }
			};

			for (size_t i = 0; i < quadVertexCount; ++i)
			{
				mesh.Vertices.push_back({ color, quadVertexPositions[i], texCoords[i], textureID });
			}
		}
		void GenerateInGuiImage(InGuiMesh& mesh, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color)
		{
			uint32_t textureID = 0;
			for (auto& pair : mesh.TexturePairs)
			{
				if (pair.RendererID == rendererID)
				{
					textureID = pair.TextureID;
				}
			}
			if (!textureID)
			{
				textureID = g_InContext->RenderData.NumTexturesInUse;
				g_InContext->RenderData.NumTexturesInUse++;
			}
			GenerateInGuiQuad(mesh, position, size, texCoord, textureID, color);
			mesh.TexturePairs.push_back({ textureID,rendererID });
		}
		std::pair<int32_t, int32_t> GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
		{
			auto& fontData = font->GetData();
			int32_t cursorX = 0, cursorY = 0;

			int32_t width = 0;
			int32_t height = 0;
			
			for (auto c : str)
			{
				auto& character = font->GetCharacter(c);
				if (width + (character.XAdvance * scale.x) >= length)
					break;

				float yOffset = (fontData.LineHeight - character.YOffset - character.Height) * scale.y;
				glm::vec2 pos = {
					cursorX + character.XOffset + position.x,
					cursorY + yOffset + position.y
				};

				glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
				glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
				glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

				mesh.Vertices.push_back({color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,textureID });
				mesh.Vertices.push_back({color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,textureID });
				mesh.Vertices.push_back({color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,textureID });
				mesh.Vertices.push_back({color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,textureID });

				if (size.y > height)
					height = size.y;


				width += character.XAdvance * scale.x;
				cursorX += character.XAdvance * scale.x;
			}
			return std::pair<int32_t, int32_t>(width, height);
		}
		std::pair<int32_t, int32_t> GenerateInGuiText(InGuiVertex* vertices, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
		{
			auto& fontData = font->GetData();
			int32_t cursorX = 0, cursorY = 0;

			int32_t width = 0;
			int32_t height = 0;
			uint32_t counter = 0;
			for (auto c : str)
			{
				auto& character = font->GetCharacter(c);
				if (width + (character.XAdvance * scale.x) >= length)
					break;

				float yOffset = (fontData.LineHeight - character.YOffset - character.Height) * scale.y;
				glm::vec2 pos = {
					cursorX + character.XOffset + position.x,
					cursorY + yOffset + position.y
				};

				glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
				glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
				glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

				vertices[counter+0] = { color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,textureID };
				vertices[counter+1] = { color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,textureID };
				vertices[counter+2] = { color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,textureID };
				vertices[counter+3] = { color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,textureID };
				counter += 4;

				if (size.y > height)
					height = size.y;


				width += character.XAdvance * scale.x;
				cursorX += character.XAdvance * scale.x;
			}
			return std::pair<int32_t, int32_t>(width, height);
		}
		void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, size_t offset, size_t count)
		{
			for (size_t i = offset; i < count + offset; ++i)
			{
				vertices[i].Position.x += position.x;
				vertices[i].Position.y += position.y;
			}
		}
		void HandleInputText(std::string& text)
		{
			InGuiFrameData& frameData = g_InContext->FrameData;
			size_t currentSize = text.size();
			switch (frameData.PressedKey)
			{
			case ToUnderlying(KeyCode::XYZ_KEY_BACKSPACE):
				if (!text.empty())
					text.pop_back();
				return;
			case ToUnderlying(KeyCode::XYZ_KEY_0):
				text.push_back('0');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_1):
				text.push_back('1');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_2):
				text.push_back('2');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_3):
				text.push_back('3');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_4):
				text.push_back('4');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_5):
				text.push_back('5');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_6):
				text.push_back('6');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_7):
				text.push_back('7');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_8):
				text.push_back('8');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_9):
				text.push_back('9');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_PERIOD):
				text.push_back('.');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_A):
				text.push_back('a');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_B):
				text.push_back('b');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_C):
				text.push_back('c');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_D):
				text.push_back('d');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_E):
				text.push_back('e');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_F):
				text.push_back('f');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_G):
				text.push_back('g');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_H):
				text.push_back('h');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_I):
				text.push_back('i');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_J):
				text.push_back('j');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_K):
				text.push_back('k');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_L):
				text.push_back('l');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_M):
				text.push_back('m');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_O):
				text.push_back('o');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_P):
				text.push_back('p');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_Q):
				text.push_back('q');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_R):
				text.push_back('r');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_S):
				text.push_back('s');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_T):
				text.push_back('t');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_U):
				text.push_back('u');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_V):
				text.push_back('v');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_W):
				text.push_back('w');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_X):
				text.push_back('x');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_Y):
				text.push_back('y');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_Z):
				text.push_back('z');
				break;
			}	

			// Shift / Capslock
			if ((frameData.KeyMode == 1 || frameData.CapslockEnabled) && currentSize < text.size())
			{
				text[currentSize] = std::toupper(text[currentSize]);
			}
		}
		void HandleInputNumber(std::string& text)
		{
			InGuiFrameData& frameData = g_InContext->FrameData;

			switch (frameData.PressedKey)
			{
			case ToUnderlying(KeyCode::XYZ_KEY_BACKSPACE):
				if (!text.empty())
					text.pop_back();
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_0):
				text.push_back('0');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_1):
				text.push_back('1');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_2):
				text.push_back('2');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_3):
				text.push_back('3');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_4):
				text.push_back('4');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_5):
				text.push_back('5');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_6):
				text.push_back('6');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_7):
				text.push_back('7');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_8):
				text.push_back('8');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_9):
				text.push_back('9');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_PERIOD):
				text.push_back('.');
				break;
			}
		}
	}
}