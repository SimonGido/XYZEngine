#pragma once
#include "XYZ/ECS/Component.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum class CanvasRenderMode
	{
		ScreenSpace,
		RenderModes
	};

	struct Canvas : public ECS::Type<Canvas>
	{
		Canvas() = default;
		Canvas(CanvasRenderMode mode)
			:
			RenderMode(mode)
		{}

		CanvasRenderMode RenderMode;
	};
}