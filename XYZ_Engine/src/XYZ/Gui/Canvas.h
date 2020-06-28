#pragma once
#include "XYZ/Physics/PhysicsComponent.h"
#include "XYZ/Utils/DataStructures/Tree.h"

namespace XYZ {

	struct Canvas : public Type<Canvas>
	{
		Canvas(bool overlay,bool receiveEvents)
			: Overlay(overlay),ReceiveEvents(receiveEvents)
		{}

		bool Overlay;
		bool ReceiveEvents;
	};
}