#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	
	Button::Button(const glm::vec4& clickColor, const glm::vec4& hooverColor)
		:
		ClickColor(clickColor), HooverColor(hooverColor)
	{
		State<ButtonState::NumStates>& clickState = Machine.CreateState();
		State<ButtonState::NumStates>& releaseState = Machine.CreateState();
		State<ButtonState::NumStates>& hooverState = Machine.CreateState();
		State<ButtonState::NumStates>& unHooverState = Machine.CreateState();

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