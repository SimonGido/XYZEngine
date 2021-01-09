#pragma once
#include "XYZ/Core/Timestep.h"

namespace XYZ {

	class Script
	{
	public:
		virtual ~Script() = default;
		virtual void OnCreate() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnDestroy() {}
	};
}