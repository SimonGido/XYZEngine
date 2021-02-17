#include "stdafx.h"
#include "Text.h"


namespace XYZ {
	Text::Text(const std::string& source, const Ref<XYZ::Font>& font,const glm::vec4& color, TextAlignment alignment)
		:
		Source(source),
		Font(font),
		Color(color),
		Alignment(alignment)
	{
	}
	
}