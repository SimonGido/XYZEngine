#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "XYZ/FSM/StateMachine.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace InputFieldState {
		enum InputFieldState
		{
			Selected,
			Released,
			Hoovered,
			UnHoovered,
			NumStates
		};
	}
	struct InputField : public ECS::Type<InputField>,
					    public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent>
	{
		InputField(const glm::vec4& selectColor, const glm::vec4& hooverColor, uint32_t entity, ECS::ECSManager* ecs);

		glm::vec4 SelectColor;
		glm::vec4 HooverColor;

		uint32_t TextEntity;
		ECS::ECSManager* ECS;

		StateMachine<InputFieldState::NumStates> Machine;

		template <typename T>
		static bool GetValueAs(const InputField& field, T& value);
	};
}