#include "EditorOrthographicCamera.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	namespace Editor {
		void EditorOrthographicCamera::UpdateViewProjection()
		{
			glm::mat4 viewMatrix = glm::inverse(glm::translate(glm::vec3(Position, 0.0f)));
			ViewProjectionMatrix = ProjectionMatrix * viewMatrix;
		}
	}
}
