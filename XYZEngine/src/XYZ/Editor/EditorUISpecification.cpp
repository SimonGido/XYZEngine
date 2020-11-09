#include "stdafx.h"
#include "EditorUISpecification.h"


namespace XYZ {
	ButtonSpecification::ButtonSpecification(
		const std::string& name,
		const glm::vec3& position,
		const glm::vec2& size,
		const glm::vec4& defaultColor,
		const glm::vec4& clickColor,
		const glm::vec4& hooverColor
	)
		:
		Name(name), Position(position), Size(size), DefaultColor(defaultColor), ClickColor(clickColor), HooverColor(hooverColor)
	{
	}
	CanvasSpecification::CanvasSpecification(CanvasRenderMode mode, const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
		:
		RenderMode(mode),
		Position(position),
		Size(size),
		Color(color)
	{
	}
}