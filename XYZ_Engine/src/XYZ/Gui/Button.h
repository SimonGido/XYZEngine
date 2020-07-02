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
				   public EventSystem<ClickEvent,ReleaseEvent,HooverEvent>,
				   Type<Button>
	{
	public:
		Button();

		void SetHighlightColor(const glm::vec4& color);
		void SetPressColor(const glm::vec4& color);
		
		void OnClick(const ClickEvent& event);
		void OnRelease(const ReleaseEvent& event);
		void OnHoover(const HooverEvent& event);
	

		virtual void OnEvent(Event& event) override;
	private:
		template <typename Event>
		void receiveEvent(Event& e)
		{
			m_StateMachine.Handle(e);
		}
	private:
		glm::vec4 m_HighlightColor;
		glm::vec4 m_PressColor;
		
	private:	
		struct Released;
		struct Hoovered;
		struct Clicked;

		struct Clicked : public Setup<Default<Nothing>,
									  On<ReleaseEvent, TransitionTo<Released>>>
		{
			Clicked(Button* button);
			void OnEnter(const ClickEvent& e);

		private:
			Button* Btn;
		};

		struct Released : public Setup<Default<Nothing>,
									   On<HooverEvent, TransitionTo<Hoovered>>>
		{
			Released(Button* button);
			void OnEnter(const ReleaseEvent& );

		private:
			Button* Btn;
		};
		
		struct Hoovered : public Setup<Default<Nothing>,
									   On<ClickEvent, TransitionTo<Clicked>>,
									   On<ReleaseEvent,  TransitionTo<Released>>>
		{
			Hoovered(Button* button);
			void OnEnter(const HooverEvent& e);
			
		private:
			Button* Btn;
		};

		StateMachine <Released, Clicked, Hoovered > m_StateMachine;

	};
}