#pragma once
#include "XYZ/Core/Timestep.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		// TODO: Implement properly
		struct EditorOrthographicCamera
		{
			void UpdateViewProjection();
			void Update(Timestep ts);

			glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
			glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
			glm::vec2 Position = glm::vec2(0.0f);
			float Speed = 150.0f;
		};
	}
}