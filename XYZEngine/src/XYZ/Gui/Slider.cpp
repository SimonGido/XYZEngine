#include "stdafx.h"
#include "Slider.h"


namespace XYZ {
	Slider::Slider(const glm::vec4& hooverColor)
		:
		HooverColor(hooverColor)
	{
		State<SliderState::NumStates>& dragState = Machine.CreateState();
		State<SliderState::NumStates>& releaseState = Machine.CreateState();


		dragState.AllowTransition(releaseState.GetID());
		releaseState.AllowTransition(dragState.GetID());
		Machine.SetDefaultState(releaseState.GetID());
	}
}