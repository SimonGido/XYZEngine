#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Event/EventSystem.h"
#include "XYZ/Event/GuiEvent.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {
	struct RectTransform : public Type<RectTransform>,
						   public EventSystem<CanvasRendererRebuildEvent> // Editor only , it fires event when canvas should be rebuilt
	{
		RectTransform(const glm::vec3& position, const glm::vec2& size)
			: WorldPosition(position), Position(position), Size(size)
		{}

		glm::vec3 WorldPosition;
		glm::vec3 Position;
		glm::vec2 Size;
		glm::vec2 Scale = glm::vec2(1.0f);


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