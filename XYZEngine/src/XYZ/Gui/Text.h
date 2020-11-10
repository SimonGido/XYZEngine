#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/Font.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum class TextAlignment
	{
		Left,
		Right,
		Center
	};

	struct Text : public Type<Text>
	{
		Text(const std::string& source, const Ref<Font>& font, const glm::vec4& color, TextAlignment alignment);
		
		std::string   Source;
		Ref<Font>     Font;
		glm::vec4	  Color;
		TextAlignment Alignment;
	};
}