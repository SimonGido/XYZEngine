#include "stdafx.h"
#include "InGuiElement.h"




namespace XYZ {
	

	IGElement::IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		Position(position), 
		Size(size), 
		Color(color),
		FrameColor(color)
	{
	}

	glm::vec2 IGElement::GetAbsolutePosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Position;
		}
		return Position;
	}

	IGReturnType IGElement::getAndRestartReturnType()
	{
		IGReturnType old = ReturnType;
		ReturnType = IGReturnType::None;
		return old;
	}
}