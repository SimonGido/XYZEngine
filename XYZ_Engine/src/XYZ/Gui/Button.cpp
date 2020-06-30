#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	Button::Button()
		: m_StateMachine(Released{ this }, Clicked{ this }, Hoovered{ this })
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


}