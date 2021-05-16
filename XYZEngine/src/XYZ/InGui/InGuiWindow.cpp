#include "stdafx.h"
#include "InGuiWindow.h"

namespace XYZ {	
	InGuiWindow::InGuiWindow()
		:
		ID(0),
		Flags(0),
		Position(100.0f),
		Size(300.0f),
		Scroll(0.0f),
		PanelHeight(25.0f),
		LabelOffset(5.0f)
	{
	}

	void InGuiWindow::PushItselfToDrawlist(const glm::vec4& color, const InGuiConfig& config)
	{
		const Ref<SubTexture>& winSubTexture = config.GetSubTexture(InGuiConfig::Window);
		const Ref<SubTexture>& panelSubTexture = config.GetSubTexture(InGuiConfig::Button);
		const Ref<SubTexture>& minimizeSubTexture = config.GetSubTexture(InGuiConfig::MinimizeButton);
		
		glm::vec4 defaultColor = config.GetColor(InGuiConfig::DefaultColor);
		if (!IS_SET(Flags, InGuiWindow::Collapsed))
		{
			DrawList.PushQuadNoClip(
				defaultColor, winSubTexture->GetTexCoords(),
				Position, Size, InGuiConfig::sc_DefaultTexture
			);
		}
	
		DrawList.PushQuadNoClip(
			color, panelSubTexture->GetTexCoords(),
			Position, { Size.x, PanelHeight }, InGuiConfig::sc_DefaultTexture
		);

		glm::vec2 buttonPosition = Position + glm::vec2(Size.x - PanelHeight, 0.0f);
		DrawList.PushQuadNoClip(
			defaultColor, minimizeSubTexture->GetTexCoords(),
			buttonPosition, { PanelHeight, PanelHeight }, InGuiConfig::sc_DefaultTexture
		);

		glm::vec2 textPosition = glm::vec2(Position.x + LabelOffset, Position.y + PanelHeight);
		textPosition.y -= std::floor((PanelHeight - config.GetFont()->GetLineHeight()) / 2.0f);
		DrawList.PushTextNoClip(Name.c_str(), textPosition, defaultColor, InGuiConfig::sc_FontTexture, config.GetFont());
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

		if (mousePosition.y > Position.y + Size.x - sc_ResizeThresholdY)
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
		if (windowRectangle.Overlaps(mousePosition))
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
}