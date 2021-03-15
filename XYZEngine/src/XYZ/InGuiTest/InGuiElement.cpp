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
		ElementType = IGElementType::Window;
	}

	bool IGWindow::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(AbsolutePosition, Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			glm::vec2 minButtonPos = { Position.x + Size.x - IGWindow::PanelHeight, Position.y };
			if (Position.y + IGWindow::PanelHeight >= mousePosition.y)
			{
				if (Helper::Collide(minButtonPos, { IGWindow::PanelHeight, IGWindow::PanelHeight }, mousePosition))
				{
					Flags ^= IGWindow::Flags::Collapsed;
				}
				else
				{
					Flags |= IGWindow::Flags::Moved;
				}
				return true;
			}
			else
			{
				// TODO: Resizing
			}
		}
		return false;
	}

	bool IGWindow::OnLeftRelease(const glm::vec2& mousePosition)
	{
		Flags &= ~IGWindow::Flags::Moved;
		if (Helper::Collide(AbsolutePosition, Size, mousePosition))
		{
			ReturnType = IGReturnType::Released;
			return true;
		}
		return false;
	}

	bool IGWindow::OnMouseMove(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(AbsolutePosition, Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			return true;
		}
		return false;
	}


	IGButton::IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{}
	IGCheckbox::IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
	IGSlider::IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
}