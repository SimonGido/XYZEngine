#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	Button::Button()
		:
		m_HighlightColor(glm::vec4(1)),
		m_PressColor(glm::vec4(1)),
		m_StateMachine(Hoovered{ this },Released{ this }, Clicked{ this })
	{
		m_StateMachine.Handle(HooverEvent{});
	}

	Button::Button(const Button& other)
		:
		m_HighlightColor(other.m_HighlightColor),
		m_PressColor(other.m_PressColor),
		m_StateMachine(Hoovered{this}, Released{ this }, Clicked{ this })
	{
	}
	
	void Button::SetHighlightColor(const glm::vec4& color)
	{
		m_HighlightColor = color;
	}
	void Button::SetPressColor(const glm::vec4& color)
	{
		m_PressColor = color;
	}
	
	void Button::OnClick(const ClickEvent& e)
	{
		Execute(ClickEvent{});
	}

	void Button::OnRelease(const ReleaseEvent& event)
	{
		Execute(ReleaseEvent{});
	}

	void Button::OnHoover(const HooverEvent& event)
	{
		Execute(HooverEvent{});
	}

	void Button::OnEvent(Event& event)
	{
		//if (event.GetEventType() == ClickEvent::GetStaticType())
		//{
		//	m_StateMachine = StateMachine(Hoovered{ this }, Released{ this }, Clicked{ this });
		//	m_StateMachine.Handle(ClickEvent{});
		//	//Execute(ClickEvent{});
		//}
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<ClickEvent>(Hook(&Button::receiveEvent<ClickEvent>, this));
		dispatcher.Dispatch<ReleaseEvent>(Hook(&Button::receiveEvent<ReleaseEvent>, this));
		dispatcher.Dispatch<HooverEvent>(Hook(&Button::receiveEvent<HooverEvent> , this));
	}

	Button::Clicked::Clicked(Button* button)
		: Btn(button)
	{}

	void Button::Clicked::OnEnter(const ClickEvent& e)
	{
		Btn->OnClick(e);
		std::cout << "Clicked" << std::endl;
	}

	Button::Released::Released(Button* button)
		: Btn(button)
	{
	}

	void Button::Released::OnEnter(const ReleaseEvent& e)
	{
		Btn->OnRelease(e);
		std::cout << "Released" << std::endl;
	}

	Button::Hoovered::Hoovered(Button* button)
		: Btn(button)
	{
	}

	void Button::Hoovered::OnEnter(const HooverEvent& e)
	{
		Btn->OnHoover(e);
		std::cout << "Hoovered" << std::endl;
	}

}