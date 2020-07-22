#include "stdafx.h"
#include "InGuiHelper.h"

#include "XYZ/Core/Application.h"


namespace XYZ {
	namespace InGui {

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
			}

			// Subtract from position widget size y
			position.y -= uiSize.y;
			// Add to position window space offset and offset
			position += g_InContext->InGuiData.WindowSpaceOffset + glm::vec2{ offset.x,-offset.y };


			g_InContext->InGuiData.WindowSpaceOffset.x += uiSize.x + offset.x;

			return position;
		}

		bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			glm::vec2 offset = { g_InContext->InGuiData.WindowSizeX / 2,g_InContext->InGuiData.WindowSizeY / 2 };

			return (pos.x + size.x >= point.x - offset.x &&
				pos.x <= point.x - offset.x &&
				pos.y + size.y >= offset.y - point.y &&
				pos.y <= offset.y - point.y);
		}

		void HandleResize(glm::vec2& position, glm::vec2& size, const glm::vec2& mousePos)
		{
			glm::vec2 offset = { 10,10 };
			auto& app = Application::Get();

			// Right side
			if (mousePos.x >= position.x + size.x - offset.x)
			{
				app.GetWindow().SetCursor(WindowCursor::XYZ_HRESIZE_CURSOR);
				//window.Size.x = mousePos.x - window.Position.x;
			}

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