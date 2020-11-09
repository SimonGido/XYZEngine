#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	
	Button::Button(const glm::vec4& defaultColor, const glm::vec4& clickColor, const glm::vec4& hooverColor)
		:
		DefaultColor(defaultColor), ClickColor(clickColor), HooverColor(hooverColor)
	{
	}
}