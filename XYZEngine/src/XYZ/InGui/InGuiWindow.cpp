#include "stdafx.h"
#include "InGuiWindow.h"

#include "InGuiUtil.h"
#include "InGui.h"

namespace XYZ {	
	InGuiWindow::InGuiWindow()
		:
		ClipID(0),
		EditFlags(0),
		StyleFlags(0),
		Position(100.0f),
		Size(300.0f),
		Scroll(0.0f),
		IsActive(true),
		FrameData(this),
		Axis(AxisPlacement::Vertical),
		ScrollBarX(false),
		ScrollBarY(false),
		ScrollBarSize(10.0f)
	{
		StyleFlags |= InGuiWindowStyleFlags::PanelEnabled;
		StyleFlags |= InGuiWindowStyleFlags::ScrollEnabled;
	}

	void InGuiWindow::PushItselfToDrawlist(const glm::vec4& color)
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		const Ref<SubTexture>& winSubTexture = config.SubTextures[InGuiConfig::Window];
		const Ref<SubTexture>& panelSubTexture = config.SubTextures[InGuiConfig::Button];
		const Ref<SubTexture>& minimizeSubTexture = config.SubTextures[InGuiConfig::MinimizeButton];

		glm::vec4 defaultColor = config.Colors[InGuiConfig::DefaultColor];
		if (!IS_SET(EditFlags, InGuiWindowEditFlags::Collapsed))
		{
			DrawList.PushQuad(
				defaultColor, winSubTexture->GetTexCoords(),
				Position, Size, InGuiConfig::sc_DefaultTexture, 0
			);
		}
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::PanelEnabled))
		{
			DrawList.PushQuad(
				color, panelSubTexture->GetTexCoords(),
				Position, { Size.x, config.PanelHeight }, InGuiConfig::sc_DefaultTexture, 0
			);

			glm::vec2 buttonPosition = Position + glm::vec2(Size.x - config.PanelHeight, 0.0f);
			DrawList.PushQuad(
				defaultColor, minimizeSubTexture->GetTexCoords(),
				buttonPosition, { config.PanelHeight, config.PanelHeight }, InGuiConfig::sc_DefaultTexture, 0
			);

			glm::vec2 textPosition = glm::vec2(Position.x + config.LabelOffset, Position.y + config.PanelHeight);
			textPosition.y -= std::floor((config.PanelHeight - config.Font->GetLineHeight()) / 2.0f);
			Util::GenerateTextMeshClipped(
				Name.c_str(), config.Font, defaultColor, 
				textPosition, DrawList.m_Quads, 
				InGuiConfig::sc_FontTexture, 0, 
				{ Size.x - config.PanelHeight, config.PanelHeight }
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

	void InGuiWindow::PushTextNotClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
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
			0
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

	void InGuiWindow::PushQuadNotClipped(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawList.PushQuad(
			color, texCoord,
			pos, size,
			textureID, 0
		);
	}

	void InGuiWindow::PushTextClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
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
			DrawList.m_QuadsOverlay, InGuiConfig::sc_FontTexture,
			ClipID, size
		);
	}

	void InGuiWindow::PushTextNotClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
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
			DrawList.m_QuadsOverlay, InGuiConfig::sc_FontTexture,
			0
		);
	}

	void InGuiWindow::PushTextOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
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
			DrawList.m_QuadsOverlay, InGuiConfig::sc_FontTexture,
			ClipID
		);
	}

	void InGuiWindow::PushQuadOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawList.PushQuadOverlay(
			color, texCoord,
			pos, size,
			textureID, ClipID
		);
	}

	void InGuiWindow::PushQuadNotClippedOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawList.PushQuadOverlay(
			color, texCoord,
			pos, size,
			textureID, 0
		);
	}

	bool InGuiWindow::NextWidgetClipped(const glm::vec2& size)
	{
		InGuiRect rect(FrameData.CursorPos, FrameData.CursorPos + size);
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::ScrollEnabled))
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
		
		old -= Scroll;
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
			EditFlags |= InGuiWindowEditFlags::ResizeLeft;
			result = true;
		}
		else if (mousePosition.x > Position.x + Size.x - sc_ResizeThresholdX)
		{
			EditFlags |= InGuiWindowEditFlags::ResizeRight;
			result = true;
		}

		if (mousePosition.y > Position.y + Size.y - sc_ResizeThresholdY)
		{
			EditFlags |= InGuiWindowEditFlags::ResizeBottom;
			result = true;
		}
		return result;
	}

	bool InGuiWindow::HandleResize(const glm::vec2& mousePosition)
	{
		bool result = false;
		if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeRight))
		{
			Size.x = mousePosition.x - Position.x;
			result = true;
		}
		else if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeLeft))
		{
			float oldX = Position.x;
			Position.x = mousePosition.x;
			Size.x -= Position.x - oldX;
		}
		if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeBottom))
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
			EditFlags |= InGuiWindowEditFlags::Hoovered;
			InGuiRect panelRectangle = PanelRect();
			return panelRectangle.Overlaps(mousePosition);
		}
		else
		{
			EditFlags &= ~InGuiWindowEditFlags::Hoovered;
		}
		return false;
	}

	bool InGuiWindow::HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset)
	{
		if (IS_SET(EditFlags, InGuiWindowEditFlags::Moving))
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
		
		// Backup cursor
		glm::vec2 oldCursor = FrameData.CursorPos;
		InGuiRect rect = ClipRect();
		if (ScrollBarY)
		{
			SetCursorPosition(glm::vec2(
				rect.Max.x - ScrollBarSize.y - InGuiWindow::sc_ResizeThresholdX,
				rect.Min.y + InGuiWindow::sc_ResizeThresholdY)
			);
			float yScroll = Scroll.y;
			float ySize = rect.Max.y - rect.Min.y - (2.0f * InGuiWindow::sc_ResizeThresholdY);
			Scroll.y = 0.0f;
			InGui::VSliderFloat(
				"", glm::vec2(ScrollBarSize.y, ySize),
				yScroll, 0.0f, FrameData.ScrollMax.y, nullptr
			);
			Scroll.y = yScroll;
		}
		else if (ScrollBarX)
		{
			SetCursorPosition(glm::vec2(
				rect.Min.x + InGuiWindow::sc_ResizeThresholdX,
				rect.Max.y - ScrollBarSize.y - InGuiWindow::sc_ResizeThresholdY)
			);
			float xScroll = Scroll.x;
			float xSize = rect.Max.x - rect.Min.x - (2.0f * InGuiWindow::sc_ResizeThresholdX);
			Scroll.x = 0.0f;
			InGui::SliderFloat(
				"", glm::vec2(xSize, ScrollBarSize.y),
				xScroll, 0.0f, FrameData.ScrollMax.x, nullptr
			);
			Scroll.x = xScroll;
		}
		// Restore cursor
		SetCursorPosition(oldCursor);
	}

	bool InGuiWindow::IsFocused() const
	{
		return this == InGui::GetContext().m_FocusedWindow;
	}

	InGuiRect InGuiWindow::PanelRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		return InGuiRect(Position, Position + glm::vec2(Size.x, config.PanelHeight));;
	}

	InGuiRect InGuiWindow::MinimizeRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		return InGuiRect(
			Position + glm::vec2(Size.x - config.PanelHeight, 0.0f), 
			Position + glm::vec2(Size.x,  config.PanelHeight)
		);
	}

	InGuiRect InGuiWindow::ClipRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 min = { Position.x, Position.y + config.PanelHeight };
		InGuiRect rect(min, min + glm::vec2(Size.x, Size.y - config.PanelHeight));
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			rect.Min.y += config.MenuBarHeight;
		return rect;
	}

	InGuiRect InGuiWindow::ClipRect(uint32_t viewportHeight) const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 min = { Position.x, (float)viewportHeight - Position.y - Size.y };
		glm::vec2 max = min + glm::vec2(Size.x, Size.y - config.PanelHeight);	
		InGuiRect rect(min, max);

		return rect;
	}	
	InGuiWindowFrameData::InGuiWindowFrameData(InGuiWindow* window)
		:
		CursorPos(window->Position.x, window->Position.y + InGui::GetContext().m_Config.PanelHeight),
		ScrollMax(0.0f),
		MenuOpen(nullptr)
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		Params.RowWidth = 0.0f;
		CursorPos += config.WindowPadding;
		if (IS_SET(window->StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			CursorPos.y += config.MenuBarHeight;
	}
}