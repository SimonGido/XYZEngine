#pragma once
#include "XYZ/ECS/Component.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct RectTransform : public Type<RectTransform>
	{
		RectTransform(const glm::vec3& position, const glm::vec2& size)
			: WorldPosition(position), Position(position), Size(size)
		{}

		glm::vec3 WorldPosition;
		glm::vec3 Position;
		glm::vec2 Size;
	};
}