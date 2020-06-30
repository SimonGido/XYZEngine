#pragma once
#include "XYZ/FSM/StateMachine.h"
#include "XYZ/FSM/TransitionTo.h"
#include "XYZ/FSM/Setup.h"

namespace XYZ {

	struct Clicked;
	struct Released;
	struct Checked;
	struct UnChecked;
	struct Hoovered;

	struct Release {};
	struct Click {};
	struct Hoover{};
	struct UnHoover{};


	struct Clicked : public Setup<Default<Nothing>,
							On<Release,TransitionTo<Released>>>								
	{

	};


	struct Released : public Setup<Default<Nothing>,
							 On<Click, TransitionTo<Clicked>>,
							 On<Hoover, TransitionTo<Hoovered>>>
	{
	};


	struct Hoovered : public Setup<Default<Nothing>,
							On<Click, TransitionTo<Clicked>>,
							On<UnHoover, TransitionTo<Released>>>
	{
	};

}