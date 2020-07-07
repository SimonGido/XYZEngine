#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/FSM/StateMachine.h"
#include "XYZ/FSM/Setup.h"
#include "XYZ/FSM/TransitionTo.h"

#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "Widget.h"

#include <glm/glm.hpp>

#include <functional>

namespace XYZ {
	class Button : public Widget,
				   public EventSystem<ClickEvent,ReleaseEvent,HooverEvent,UnHooverEvent>,
				   public Type<Button>
	{
	public:
		Button();


		virtual void OnEvent(Event& event) override;
		virtual WidgetType GetWidgetType() override { return WidgetType::Button; }

	private:
		template <typename Event>
		bool receiveEvent(Event& e)
		{
			return m_StateMachine.Handle(e);
		}

		
	private:	
		struct Released;
		struct Hoovered;
		struct UnHoovered;
		struct Clicked;

		struct Clicked : public Setup<Default<Nothing>,
									  On<ReleaseEvent, TransitionTo<Released>>>
		{
			
		};

		struct Released : public Setup<Default<Nothing>,
									   On<HooverEvent, TransitionTo<Hoovered>>,
									   On<ClickEvent,TransitionTo<Clicked>>>
		{
			
		};
		
		struct Hoovered : public Setup<Default<Nothing>,
									   On<ClickEvent, TransitionTo<Clicked>>,
									   On<ReleaseEvent,  TransitionTo<Released>>,
									   On<UnHooverEvent, TransitionTo<UnHoovered>>>
		{
		};

		struct UnHoovered : public Setup<Default<Nothing>,
										 On<HooverEvent, TransitionTo<Hoovered>>>
		{
		};

		StateMachine <Hoovered,Released, Clicked, UnHoovered> m_StateMachine;

	};
}