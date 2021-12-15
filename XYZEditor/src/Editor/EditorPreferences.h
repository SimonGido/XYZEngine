#pragma once
#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		struct EditorPreferences
		{
			glm::vec4 BoundingBoxColor = glm::vec4(1.0f);
			glm::vec4 Collider2DColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		};

	}
}