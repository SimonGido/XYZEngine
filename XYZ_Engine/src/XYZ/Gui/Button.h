#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/FSM/StateMachine.h"
#include "GuiStates.h"

#include <glm/glm.hpp>

#include <functional>

namespace XYZ {
	class Button : public Type<Button>
	{
	public:
		using Callback = std::function<void()>;

		Button();
		void AddCallback(const Callback& callback);
		void RemoveCallback(size_t index);
		void SetHighlightColor(const glm::vec4& color);
		void SetPressColor(const glm::vec4& color);
		void OnClick();


		template <typename Event>
		void ReceiveEvent(Event& e)
		{
			m_StateMachine.Handle(e);
		}
	
	private:
		glm::vec4 m_HighlightColor;
		glm::vec4 m_PressColor;
		std::vector<Callback> m_Callbacks;
		
	private:
		
		struct Released;
		struct Hoovered;

		struct Clicked : public Setup<Default<Nothing>,
								On<Release, TransitionTo<Released>>>
		{
			Clicked(Button* button)
				: Btn(button)
			{}

			void OnEnter(const Release&)
			{
				Btn->OnClick();
				std::cout << "Clicked" << std::endl;
			}
			Button* Btn;
		};

		struct Released : public Setup<Default<Nothing>,
								 On<Hoover, TransitionTo<Hoovered>>>
		{
			Released(Button* button)
				: Btn(button)
			{}

			void OnEnter(const Release&)
			{
				std::cout << "Released" << std::endl;
			}
			Button* Btn;
		};
		
		struct Hoovered : public Setup<Default<Nothing>,
								 On<Click, TransitionTo<Clicked>>,
								 On<UnHoover, TransitionTo<Released>>>
		{
			Hoovered(Button* button)
				: Btn(button)
			{}

			void OnEnter(const Hoover&)
			{
				std::cout << "Hoovered" << std::endl;
			}
			Button* Btn;
		};

		StateMachine <Released, Clicked, Hoovered > m_StateMachine;

	};
}