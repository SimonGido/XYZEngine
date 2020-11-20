#pragma once
#include "XYZ/ECS/Component.h"
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
			UnHoovered,
			NumStates
		};
	}

	class Button : public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent>,
				   public ECS::Type<Button>
	{
	public:
		Button(const glm::vec4& clickColor, const glm::vec4& hooverColor);

		
		glm::vec4   ClickColor;
		glm::vec4   HooverColor;

		StateMachine<ButtonState::NumStates> Machine;
	};
}