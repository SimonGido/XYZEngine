#include "EditorOrthographicCamera.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	namespace Editor {
		void EditorOrthographicCamera::UpdateViewProjection()
		{
			glm::mat4 viewMatrix = glm::inverse(glm::translate(glm::vec3(Position, 0.0f)));
			ViewProjectionMatrix = ProjectionMatrix * viewMatrix;
		}
		void EditorOrthographicCamera::Update(Timestep ts)
		{
            if (Input::IsKeyPressed(KeyCode::KEY_LEFT))
            {
                Position.x -= Speed * ts;
                UpdateViewProjection();
            }
            if (Input::IsKeyPressed(KeyCode::KEY_RIGHT))
            {
                Position.x += Speed * ts;
                UpdateViewProjection();
            }
            if (Input::IsKeyPressed(KeyCode::KEY_UP))
            {
                Position.y += Speed * ts;
                UpdateViewProjection();
            }
            if (Input::IsKeyPressed(KeyCode::KEY_DOWN))
            {
                Position.y -= Speed * ts;
                UpdateViewProjection();
            }
		}
	}
}
