#pragma once
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {
	
	class LayoutGroup : public Type<LayoutGroup>
	{
	public:
		LayoutGroup() {};

		float WidthSpacing = 0.0f;
		float HeightSpacing = 0.0f;

		float LeftPadding = 0.0f;
		float RightPadding = 0.0f;
		float TopPadding = 0.0f;
		float BottomPadding = 0.0f;
	};
}