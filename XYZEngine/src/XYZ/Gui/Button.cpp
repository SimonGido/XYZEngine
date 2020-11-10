#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	
	Button::Button(const glm::vec4& defaultColor, const glm::vec4& clickColor, const glm::vec4& hooverColor)
		:
		DefaultColor(defaultColor), ClickColor(clickColor), HooverColor(hooverColor)
	{
		State& clickState = Machine.CreateState();
		State& releaseState = Machine.CreateState();
		State& hooverState = Machine.CreateState();
		State& unHooverState = Machine.CreateState();

		clickState.AllowTransition(releaseState.GetID());
		releaseState.AllowTransition(clickState.GetID());
		releaseState.AllowTransition(hooverState.GetID());
		unHooverState.AllowTransition(hooverState.GetID());
		hooverState.AllowTransition(clickState.GetID());
		hooverState.AllowTransition(hooverState.GetID());
		hooverState.AllowTransition(unHooverState.GetID());
		Machine.SetDefaultState(unHooverState.GetID());
	}
}