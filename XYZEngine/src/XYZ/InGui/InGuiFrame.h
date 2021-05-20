#pragma once
#include "InGuiWindow.h"

namespace XYZ {
	struct InGuiFrame
	{
		InGuiFrame();

		InGuiWindow*		 CurrentWindow;
		glm::vec2			 MovedWindowOffset;
		float				 CurrentMenuWidth;
		std::vector<InGuiID> MenuItems;
	};
}