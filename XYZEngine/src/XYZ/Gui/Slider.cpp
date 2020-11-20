#include "stdafx.h"
#include "Slider.h"


namespace XYZ {
	Slider::Slider(const glm::vec4& hooverColor)
		:
		HooverColor(hooverColor)
	{
		State<SliderState::NumStates>& clickState = Machine.CreateState();
		State<SliderState::NumStates>& releaseState = Machine.CreateState();
		State<SliderState::NumStates>& hooverState = Machine.CreateState();
		State<SliderState::NumStates>& unHooverState = Machine.CreateState();

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