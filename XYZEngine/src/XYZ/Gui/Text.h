#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/Font.h"

namespace XYZ {

	enum class TextAlignment
	{
		Left,
		Right,
		Center
	};

	struct Text : public Type<Text>
	{
		Text(const std::string& source, const Ref<Font>& font, TextAlignment alignment);
		
		std::string   Source;
		Ref<Font>     Font;
		TextAlignment Alignment;
	};
}