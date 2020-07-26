#include "stdafx.h"
#include "InGuiHelper.h"

#include "XYZ/Core/Application.h"


namespace XYZ {
	namespace InGui {

		static enum Color
		{
			RedToGreen,
			GreenToBlue,
			BlueToRed,
			None
		};

		glm::vec2 MouseToWorld(const glm::vec2& point)
		{
			glm::vec2 offset = { g_InContext->InGuiData.WindowSizeX / 2,g_InContext->InGuiData.WindowSizeY / 2 };
			return { point.x - offset.x, offset.y - point.y };
		}

		glm::vec2 HandleWindowSpacing(const glm::vec2& uiSize)
		{
			XYZ_ASSERT(g_InContext->InGuiData.CurrentWindow, "Missing begin call");

			// Set position to the position of current window
			glm::vec2 position = g_InContext->InGuiData.CurrentWindow->Position;
			glm::vec2 offset = { 10, 10 };

			// Find the highest widget in row
			if (uiSize.y > g_InContext->InGuiData.MaxHeightInRow)
				g_InContext->InGuiData.MaxHeightInRow = uiSize.y;


			// If widget position is going to be outside of the window
			if (g_InContext->InGuiData.CurrentWindow->Size.x <= g_InContext->InGuiData.WindowSpaceOffset.x + uiSize.x)
			{
				// Set window offset x to zero
				g_InContext->InGuiData.WindowSpaceOffset.x = 0.0f;

				// Subtract the highest widget in row and offset y from window offset y 
				g_InContext->InGuiData.WindowSpaceOffset.y -= g_InContext->InGuiData.MaxHeightInRow + offset.y;

				g_InContext->InGuiData.MaxHeightInRow = uiSize.y;
			}

			// Subtract from position widget size y
			position.y -= uiSize.y;
			// Add to position window space offset and offset
			position += g_InContext->InGuiData.WindowSpaceOffset + glm::vec2{ offset.x,-offset.y };


			g_InContext->InGuiData.WindowSpaceOffset.x += uiSize.x + offset.x;

			return position;
		}

		glm::vec4 CalculatePixelColor(const glm::vec4& pallete, const glm::vec2& position, const glm::vec2& size)
		{
			glm::vec2 pos = g_InContext->InGuiData.MousePosition - position;
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
			float pos = g_InContext->InGuiData.MousePosition.x - position.x;
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
			return (pos.x + size.x >= point.x &&
					pos.x		   <= point.x &&
					pos.y + size.y >=  point.y &&
					pos.y		   <= point.y);
		}

		bool DetectResize(const InGuiWindow& window)
		{
			glm::vec2 offset = { 10,10 };
			auto& mousePos = g_InContext->InGuiData.MousePosition;
		
			// Right side
			if (mousePos.x >= window.Position.x + window.Size.x - offset.x)
				g_InContext->InGuiData.Flags |= WindowRightResize;

			// Left side
			if (mousePos.x <= window.Position.x + offset.x)
				g_InContext->InGuiData.Flags |= WindowLeftResize;


			// Bottom side
			if (mousePos.y <= window.Position.y + offset.y )
				g_InContext->InGuiData.Flags |= WindowBottomResize;

			// Top side
			if (mousePos.y >= window.Position.y + window.Size.y - offset.y + InGuiWindow::PanelSize)
				g_InContext->InGuiData.Flags |= WindowTopResize;


			if (g_InContext->InGuiData.Flags 
				& (WindowRightResize | WindowLeftResize | WindowBottomResize | WindowTopResize))
			{
				return true;
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

			if (Collide(minButtonPos, minButtonSize, g_InContext->InGuiData.MousePosition))
			{
				return true;
			}
			return false;
		}

	
		void HandleMouseInput(InGuiWindow& window)
		{
			if (Collide(window.Position, { window.Size.x,window.Size.y + InGuiWindow::PanelSize }, g_InContext->InGuiData.MousePosition)
				&& !(g_InContext->InGuiData.Flags & ClickHandled))
			{
				window.Flags |= Hoovered;

				if (g_InContext->InGuiData.Flags & RightMouseButtonDown)
				{
					if (DetectResize(window))
					{
						window.Flags |= Resized;
						g_InContext->InGuiData.Flags |= ClickHandled;
					}
					else if (!(window.Flags & Moved))
					{
						window.Flags |= Moved;
						g_InContext->InGuiData.Flags |= ClickHandled;
						g_InContext->InGuiData.ModifiedWindowMouseOffset = g_InContext->InGuiData.MousePosition - window.Position - glm::vec2{ 0, window.Size.y };
					}
				}
				else if (g_InContext->InGuiData.Flags & LeftMouseButtonDown)
				{		
					if (DetectCollapse(window))
					{
						window.Flags ^= Collapsed;
						g_InContext->InGuiData.Flags |= ClickHandled;
					}
				}
			}
			else
			{
				window.Flags &= ~Hoovered;
			}
		}

		void HandleResize(InGuiWindow& window)
		{
			auto& app = Application::Get();
			auto& mousePos = g_InContext->InGuiData.MousePosition;

			if ((g_InContext->InGuiData.Flags & RightMouseButtonDown) 
			 && (window.Flags & Resized)
			 && !(window.Flags & Docked))
			{
				if (g_InContext->InGuiData.Flags & WindowRightResize)
				{
					app.GetWindow().SetCursor(WindowCursor::XYZ_HRESIZE_CURSOR);
					window.Size.x = mousePos.x - window.Position.x;
				}
				else if (g_InContext->InGuiData.Flags & WindowLeftResize)
				{
					app.GetWindow().SetCursor(WindowCursor::XYZ_HRESIZE_CURSOR);
					window.Size.x = window.Position.x + window.Size.x - mousePos.x;
					window.Position.x = mousePos.x;
				}


				if (g_InContext->InGuiData.Flags & WindowBottomResize)
				{
					app.GetWindow().SetCursor(WindowCursor::XYZ_VRESIZE_CURSOR);
					window.Size.y = window.Position.y + window.Size.y - mousePos.y;
					window.Position.y = mousePos.y;
				}
				else if (g_InContext->InGuiData.Flags & WindowTopResize)
				{
					app.GetWindow().SetCursor(WindowCursor::XYZ_VRESIZE_CURSOR);
					window.Size.y = mousePos.y - window.Position.y - InGuiWindow::PanelSize;
				}
			}
			else
			{
				window.Flags &= ~Resized;
				if (!(g_InContext->InGuiData.Flags 
				& (WindowRightResize | WindowLeftResize | WindowBottomResize | WindowTopResize)))
					app.GetWindow().SetCursor(WindowCursor::XYZ_ARROW_CURSOR);
			}
			
		}

		void HandleMove(InGuiWindow& window)
		{
			if (g_InContext->InGuiData.Flags & RightMouseButtonDown)
			{
				glm::vec2 pos = g_InContext->InGuiData.MousePosition - g_InContext->InGuiData.ModifiedWindowMouseOffset;
				window.Position = { pos.x, pos.y - window.Size.y };
			}
			else
			{
				window.Flags &= ~(Moved);
			}
		}

		void HandleDocking(InGuiWindow& window)
		{
			static constexpr glm::vec2 dockSignSize = { 50,40 };
		}
	
		void Generate6SegmentColorRectangle(const glm::vec2& size, Vertex* buffer)
		{
			static constexpr uint32_t numSegments = 6;
			static constexpr uint32_t numVertices = numSegments * 4;
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
			buffer[8].Color =  { 0,1,0,1 };
			buffer[9].Color =  { 0,1,1,1 };
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
			//////////////////////////////
			buffer[20].Color = { 1,0,1,1 };
			buffer[21].Color = { 1,0,0,1 };
			buffer[22].Color = { 1,0,0,1 };
			buffer[23].Color = { 1,0,1,1 };
		}

		void GenerateInGuiText(InGuiText& text, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, const glm::vec4& color)
		{
			auto& fontData = font->GetData();
			int32_t cursorX = 0, cursorY = 0;

			text.Vertices.reserve(str.size() * 4);
			for (auto c : str)
			{
				auto& character = font->GetCharacter(c);
				if (text.Width + (character.XAdvance * scale.x) >= length)
					break;

				glm::vec2 pos = {
					cursorX + character.XOffset + position.x,
					cursorY + position.y
				};

				glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
				glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
				glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

				text.Vertices.push_back({ { pos.x , pos.y, 0.0f, 1.0f }, color,  coords / scaleFont });
				text.Vertices.push_back({ { pos.x + size.x, pos.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, 0)) / scaleFont });
				text.Vertices.push_back({ { pos.x + size.x, pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, character.Height)) / scaleFont });
				text.Vertices.push_back({ { pos.x ,pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(0,character.Height)) / scaleFont });

				if (size.y > text.Height)
					text.Height = size.y;


				text.Width += character.XAdvance * scale.x;
				cursorX += character.XAdvance * scale.x;
			}
		}
	}
}