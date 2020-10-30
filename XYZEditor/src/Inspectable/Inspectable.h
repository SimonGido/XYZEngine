#pragma once
#include <XYZ.h>

namespace XYZ {

	class Inspectable
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};
	};

}