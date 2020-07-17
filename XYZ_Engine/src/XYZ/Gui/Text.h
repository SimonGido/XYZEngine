#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "Font.h"

namespace XYZ {

	struct TextUI : public Type<TextUI>
	{
		TextUI(const std::string& text, const Ref<Font>& font)
			:
			Text(text),
			Font(font)
		{}
	
		std::string Text;
		Ref<Font> Font;
	};
}