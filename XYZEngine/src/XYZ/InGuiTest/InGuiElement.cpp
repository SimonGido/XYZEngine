#include "stdafx.h"
#include "InGuiElement.h"




namespace XYZ {
	namespace Helper {
		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}
	}

	IGElement::IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		AbsolutePosition(position),
		Position(position), 
		Size(size), 
		Color(color),
		FrameColor(color)
	{
	}
	
	IGWindow::IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
	
	bool IGWindow::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (IS_SET(Flags, IGWindow::Collapsed))
			return false;

		if (Helper::Collide(AbsolutePosition, Size, mousePosition))
		{
			glm::vec2 minButtonPos = { Position.x + Size.x - IGWindow::PanelHeight, Position.y };
			if (Position.y + IGWindow::PanelHeight >= mousePosition.y)
			{
				//if (Helper::Collide(minButtonPos, {}))
			}
		}
	}
}