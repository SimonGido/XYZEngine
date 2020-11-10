#include "stdafx.h"
#include "GuiSpecification.h"


namespace XYZ {
	CanvasSpecification::CanvasSpecification(CanvasRenderMode mode, const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
		:
		RenderMode(mode),
		Position(position),
		Size(size),
		Color(color)
	{
	}
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
	CheckboxSpecification::CheckboxSpecification(
		const std::string& name,
		const glm::vec3& position,
		const glm::vec2& size,
		const glm::vec4& defaultColor,
		const glm::vec4& hooverColor
	)
		:
		Name(name), Position(position), Size(size), DefaultColor(defaultColor), HooverColor(hooverColor)
	{
	}

	TextSpecification::TextSpecification(TextAlignment alignment, const std::string& source, const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
		:
		Alignment(alignment),
		Source(source),
		Position(position),
		Size(size),
		Color(color)
	{
	}
	PanelSpecification::PanelSpecification(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
		:
		Position(position),
		Size(size),
		Color(color)
	{
	}
}