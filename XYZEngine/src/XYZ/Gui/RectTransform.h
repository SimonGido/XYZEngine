#pragma once
#include "XYZ/ECS/Component.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct RectTransform : public Type<RectTransform>
	{
		RectTransform(const glm::vec3& position, const glm::vec2& scale)
			: WorldPosition(position), Position(position), Scale(scale)
		{}

		glm::vec3 WorldPosition;
		glm::vec3 Position;
		glm::vec2 Scale;

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Position)
				 * glm::scale(glm::mat4(1.0f), glm::vec3(Scale, 1.0f));
		}
		glm::mat4 GetWorldTransform() const
		{
			return glm::translate(glm::mat4(1.0f), WorldPosition)
				* glm::scale(glm::mat4(1.0f), glm::vec3(Scale, 1.0f));
		}
	};
}