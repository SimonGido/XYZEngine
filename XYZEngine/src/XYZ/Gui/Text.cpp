#include "stdafx.h"
#include "Text.h"


namespace XYZ {
	Text::Text(const std::string& source, const Ref<XYZ::Font>& font, TextAlignment alignment)
		:
		Source(source),
		Font(font),
		Alignment(alignment)
	{
	}
}