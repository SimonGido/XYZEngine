#pragma once

#include "XYZ/ECS/Component.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/FSM/StateMachine.h"


#include <glm/glm.hpp>

namespace XYZ {
	namespace CheckboxState {
		enum CheckboxState
		{
			Checked,
			UnChecked,
			Hoovered,
			UnHoovered
		};
	}

	class Checkbox : public EventSystem<CheckedEvent, UnCheckedEvent, HooverEvent, UnHooverEvent>,
		public ECS::Type<Checkbox>
	{
	public:
		Checkbox() = default;
		Checkbox(const glm::vec4& defaultColor,const glm::vec4& hooverColor);

		void OnUpdate(Timestep ts);

		glm::vec4   DefaultColor;
		glm::vec4   HooverColor;

		StateMachine Machine;
		bool Checked = false;
	};
}