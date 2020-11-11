#pragma once
#include "XYZ/EntityComponentSystem/Component.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum class CanvasRenderMode
	{
		ScreenSpace,
		RenderModes
	};

	struct Canvas : public ECS::Type<Canvas>
	{
		Canvas(CanvasRenderMode mode)
			:
			RenderMode(mode)
		{}

		CanvasRenderMode RenderMode;
	};
}