#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/InputEvent.h"

#include "XYZ/Event/EventSystem.h"
#include <glm/glm.hpp>

#include <functional>

namespace XYZ {
	class Button : public EventSystem<ClickEvent,ReleaseEvent,HooverEvent,UnHooverEvent>,
				   public Type<Button>
	{
	public:
		Button(const glm::vec4& defaultColor, const glm::vec4& clickColor, const glm::vec4& hooverColor);

		glm::vec4   DefaultColor;
		glm::vec4   ClickColor;
		glm::vec4   HooverColor;
	};
}