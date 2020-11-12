#pragma once
#include "XYZ/EntityComponentSystem/Component.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/InputEvent.h"

#include "XYZ/Event/EventSystem.h"
#include "XYZ/FSM/StateMachine.h"

#include <glm/glm.hpp>
#include <functional>

namespace XYZ {
	namespace ButtonState {
		enum ButtonState
		{
			Clicked,  
			Released, 
			Hoovered,  
			UnHoovered
		};
	}

	class Button : public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent>,
		public ECS::Type<Button>
	{
	public:
		Button() = default;
		Button(const glm::vec4& defaultColor, const glm::vec4& clickColor, const glm::vec4& hooverColor);

		glm::vec4   DefaultColor;
		glm::vec4   ClickColor;
		glm::vec4   HooverColor;

		StateMachine Machine;
	};
}