#pragma once
#include <glm/glm.hpp>

namespace XYZ {
	struct InGuiInput
	{
		enum Flags
		{
			LeftMouseButtonPressed  = BIT(0),
			RightMouseButtonPressed = BIT(1),
		};
		static constexpr int InvalidKey = -1;

		uint16_t  Flags			= 0;
		int		  KeyPressed	= InvalidKey;
		int		  KeyTyped		= InvalidKey;
		glm::vec2 Scroll		= glm::vec2(0.0f);
		glm::vec2 MousePosition = glm::vec2(0.0f);

	};
}