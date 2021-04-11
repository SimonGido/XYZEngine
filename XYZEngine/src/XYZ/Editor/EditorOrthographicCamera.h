#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		// TODO: Implement properly
		struct EditorOrthographicCamera
		{
			void UpdateViewProjection();

			glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
			glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
			glm::vec2 Position = glm::vec2(0.0f);
			float Speed = 150.0f;
		};
	}
}