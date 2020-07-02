#pragma once
#include "XYZ/Event/Event.h"

namespace XYZ {

	class Widget 
	{
	public:
		virtual ~Widget() = default;
		virtual void OnEvent(Event& event) = 0;

	};
}