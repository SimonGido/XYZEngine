#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	Button::Button()
		:
		m_HighlightColor(glm::vec4(1)),
		m_PressColor(glm::vec4(1)),
		m_StateMachine(Released{ this }, Clicked{ this }, Hoovered{ this })
	{
	}
	void Button::AddCallback(const Callback& callback)
	{
		m_Callbacks.push_back(callback);
	}
	void Button::RemoveCallback(size_t index)
	{
		XYZ_ASSERT(index < m_Callbacks.size(), "Attemptint to remove callback out of range");
		m_Callbacks.erase(m_Callbacks.begin() + index);
	}
	void Button::SetHighlightColor(const glm::vec4& color)
	{
		m_HighlightColor = color;
	}
	void Button::SetPressColor(const glm::vec4& color)
	{
		m_PressColor = color;
	}
	
	void Button::OnClick()
	{
		for (auto& callback : m_Callbacks)
			callback();
	}


	Button::Clicked::Clicked(Button* button)
		: Btn(button)
	{}

	void Button::Clicked::OnEnter(const Click&)
	{
		Btn->OnClick();
		std::cout << "Clicked" << std::endl;
	}

	Button::Released::Released(Button* button)
		: Btn(button)
	{
	}

	void Button::Released::OnEnter(const Release&)
	{
		std::cout << "Released" << std::endl;
	}

	Button::Hoovered::Hoovered(Button* button)
		: Btn(button)
	{
	}

	void Button::Hoovered::OnEnter(const Hoover&)
	{
		std::cout << "Hoovered" << std::endl;
	}

}