#include "stdafx.h"
#include "InGuiWindow.h"

#include "InGuiUtil.h"
#include "InGui.h"

namespace XYZ {	
	InGuiWindow::InGuiWindow()
		:
		ClipID(0),
		Flags(0),
		StyleFlags(0),
		Position(100.0f),
		Size(300.0f),
		Scroll(0.0f),
		PanelHeight(25.0f),
		LabelOffset(5.0f),
		IsActive(true),
		FrameData(this),
		Axis(AxisPlacement::Vertical),
		ScrollBarX(false),
		ScrollBarY(false),
		ScrollBarSize(10.0f)
	{
		StyleFlags |= PanelEnabled;
		StyleFlags |= ScrollEnabled;
	}

	void InGuiWindow::PushItselfToDrawlist(const glm::vec4& color, const InGuiConfig& config)
	{
		const Ref<SubTexture>& winSubTexture = config.SubTextures[InGuiConfig::Window];
		const Ref<SubTexture>& panelSubTexture = config.SubTextures[InGuiConfig::Button];
		const Ref<SubTexture>& minimizeSubTexture = config.SubTextures[InGuiConfig::MinimizeButton];

		glm::vec4 defaultColor = config.Colors[InGuiConfig::DefaultColor];
		if (!IS_SET(Flags, InGuiWindow::Collapsed))
		{
			DrawList.PushQuad(
				defaultColor, winSubTexture->GetTexCoords(),
				Position, Size, InGuiConfig::sc_DefaultTexture, 0
			);
		}
		if (IS_SET(StyleFlags, PanelEnabled))
		{
			DrawList.PushQuad(
				color, panelSubTexture->GetTexCoords(),
				Position, { Size.x, PanelHeight }, InGuiConfig::sc_DefaultTexture, 0
			);

			glm::vec2 buttonPosition = Position + glm::vec2(Size.x - PanelHeight, 0.0f);
			DrawList.PushQuad(
				defaultColor, minimizeSubTexture->GetTexCoords(),
				buttonPosition, { PanelHeight, PanelHeight }, InGuiConfig::sc_DefaultTexture, 0
			);

			glm::vec2 textPosition = glm::vec2(Position.x + LabelOffset, Position.y + PanelHeight);
			textPosition.y -= std::floor((PanelHeight - config.Font->GetLineHeight()) / 2.0f);
			Util::GenerateTextMeshClipped(
				Name.c_str(), config.Font, defaultColor, 
				textPosition, DrawList.m_Quads, 
				InGuiConfig::sc_FontTexture, 0, 
				{ Size.x - PanelHeight, PanelHeight }
			);
		}
	}

	void InGuiWindow::PushTextClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
		{
			uint32_t numCharacters = Util::CalculateNumCharacters(text, font, size);
			tmpTextSize = Util::CalculateTextSize(text, font, numCharacters);
		}
		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMeshClipped(
			text, font, color, textPosition,
			DrawList.m_Quads, InGuiConfig::sc_FontTexture,
			ClipID, size
		);	
	}

	void InGuiWindow::PushText(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
			tmpTextSize = Util::CalculateTextSize(text, font);

		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMesh(
			text, font, color, textPosition,
			DrawList.m_Quads, InGuiConfig::sc_FontTexture,
			ClipID
		);
	}

	void InGuiWindow::PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawList.PushQuad(
			color, texCoord,
			pos, size, 
			textureID, ClipID
		);
	}

	bool InGuiWindow::NextWidgetClipped(const glm::vec2& size)
	{
		InGuiRect rect(FrameData.CursorPos, FrameData.CursorPos + size);
		if (IS_SET(StyleFlags, ScrollEnabled))
			rect.Translate(-Scroll);
		return !(rect.Overlaps(ClipRect()));
	}

	glm::vec2 InGuiWindow::MoveCursorPosition(const glm::vec2& size)
	{
		glm::vec2 old = FrameData.CursorPos;
		const glm::vec2& padding = InGui::GetContext().m_Config.WindowPadding;
		float xScrollMax = FrameData.CursorPos.x - (Position.x + Size.x) + size.x + padding.x;
		float yScrollMax = FrameData.CursorPos.y - (Position.y + Size.y) + size.y + padding.y;
		if (Axis == AxisPlacement::Horizontal)
		{
			FrameData.Params.RowHeight = std::max(FrameData.Params.RowHeight, size.y);
			FrameData.CursorPos.x += size.x + padding.x;
		}
		else
		{
			FrameData.Params.RowWidth = std::max(FrameData.Params.RowWidth, size.x);
			FrameData.CursorPos.y += size.y + padding.y;
		}
		FrameData.ScrollMax.x = std::max(FrameData.ScrollMax.x, xScrollMax);
		FrameData.ScrollMax.y = std::max(FrameData.ScrollMax.y, yScrollMax);
		

		if (IS_SET(StyleFlags, ScrollEnabled))
		{
			old -= Scroll;
		}
		return old;
	}

	void InGuiWindow::SetCursorPosition(const glm::vec2& position)
	{
		FrameData.CursorPos = position;
	}

	bool InGuiWindow::ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision)
	{
		if (checkCollision)
		{
			InGuiRect rect = Rect();
			if (!rect.Overlaps(mousePosition))
				return false;
		}

		bool result = false;
		if (mousePosition.x < Position.x + sc_ResizeThresholdX)
		{
			Flags |= InGuiWindow::ResizeLeft;
			result = true;
		}
		else if (mousePosition.x > Position.x + Size.x - sc_ResizeThresholdX)
		{
			Flags |= InGuiWindow::ResizeRight;
			result = true;
		}

		if (mousePosition.y > Position.y + Size.y - sc_ResizeThresholdY)
		{
			Flags |= InGuiWindow::ResizeBottom;
			result = true;
		}
		return result;
	}

	bool InGuiWindow::HandleResize(const glm::vec2& mousePosition)
	{
		bool result = false;
		if (IS_SET(Flags, ResizeRight))
		{
			Size.x = mousePosition.x - Position.x;
			result = true;
		}
		else if (IS_SET(Flags, ResizeLeft))
		{
			float oldX = Position.x;
			Position.x = mousePosition.x;
			Size.x -= Position.x - oldX;
		}
		if (IS_SET(Flags, ResizeBottom))
		{
			Size.y = mousePosition.y - Position.y;
			result = true;
		}

		return result;
	}

	bool InGuiWindow::HandleHoover(const glm::vec2& mousePosition)
	{
		InGuiRect windowRectangle = Rect();
		if (IsFocused() && windowRectangle.Overlaps(mousePosition))
		{
			Flags |= InGuiWindow::Hoovered;
			InGuiRect panelRectangle = PanelRect();
			return panelRectangle.Overlaps(mousePosition);
		}
		else
		{
			Flags &= ~InGuiWindow::Hoovered;
		}
		return false;
	}

	bool InGuiWindow::HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset)
	{
		if (IS_SET(Flags, InGuiWindow::Moving))
		{
			Position = mousePosition - offset;
			return true;
		}
		return false;
	}

	void InGuiWindow::HandleScrollbars()
	{
		ScrollBarX = FrameData.ScrollMax.x > 0.0f;
		ScrollBarY = FrameData.ScrollMax.y > 0.0f;
		if (!ScrollBarX) Scroll.x = 0.0f;
		if (!ScrollBarY) Scroll.y = 0.0f;

		glm::vec2 oldCursor = FrameData.CursorPos;
		if (ScrollBarX)
		{
			SetCursorPosition(glm::vec2(
				Position.x + InGuiWindow::sc_ResizeThresholdX,
				Position.y + Size.y - ScrollBarSize.y - InGuiWindow::sc_ResizeThresholdY)
			);
			float xScroll = Scroll.x;
			Scroll.x = 0.0f;
			InGui::SliderFloat(
				"", glm::vec2(Size.x - (2.0f * InGuiWindow::sc_ResizeThresholdX), ScrollBarSize.y),
				xScroll, 0.0f, FrameData.ScrollMax.x, nullptr
			);
			Scroll.x = xScroll;
		}
		if (ScrollBarY)
		{
			SetCursorPosition(glm::vec2(
				Position.x + Size.x - ScrollBarSize.y - InGuiWindow::sc_ResizeThresholdX,
				Position.y + PanelHeight + InGuiWindow::sc_ResizeThresholdY)
			);
			float yScroll = Scroll.y;
			Scroll.y = 0.0f;
			InGui::VSliderFloat(
				"", glm::vec2(ScrollBarSize.y, Size.y - PanelHeight - (2.0f * InGuiWindow::sc_ResizeThresholdY)),
				yScroll, 0.0f, FrameData.ScrollMax.y, nullptr
			);
			Scroll.y = yScroll;
		}
		SetCursorPosition(oldCursor);
	}

	bool InGuiWindow::IsFocused() const
	{
		return this == InGui::GetContext().m_FocusedWindow;
	}

	InGuiRect InGuiWindow::MinimizeRect() const
	{
		return InGuiRect(
			Position + glm::vec2(Size.x - PanelHeight, 0.0f), 
			Position + glm::vec2(Size.x, PanelHeight)
		);
	}

	InGuiRect InGuiWindow::ClipRect() const
	{
		glm::vec2 min = { Position.x, Position.y + PanelHeight };
		InGuiRect rect(min, min + glm::vec2(Size.x, Size.y - PanelHeight));
		return rect;
	}

	InGuiRect InGuiWindow::ClipRect(uint32_t viewportHeight) const
	{
		glm::vec2 min = { Position.x, (float)viewportHeight - Position.y - Size.y };
		glm::vec2 max = min + glm::vec2(Size.x, Size.y - PanelHeight);
		InGuiRect rect(min, max);
		return rect;
	}	
	InGuiWindowFrameData::InGuiWindowFrameData(InGuiWindow* window)
		:
		CursorPos(window->Position.x, window->Position.y + window->PanelHeight),
		ScrollMax(0.0f)
	{
		Params.RowWidth = 0.0f;
		CursorPos += InGui::GetContext().m_Config.WindowPadding;
	}
}