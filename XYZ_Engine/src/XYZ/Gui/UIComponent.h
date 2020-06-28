#pragma once
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {

	struct UIComponent : public Type<UIComponent>
	{
		uint16_t Parent;
		size_t Index;
	};
}