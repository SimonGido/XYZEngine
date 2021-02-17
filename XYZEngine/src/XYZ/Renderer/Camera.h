#pragma once
#include <glm/glm.hpp>


namespace XYZ {

	enum class CameraProjectionType
	{
		Orthographic,
		Perspective
	};


	struct CameraPerspectiveProperties
	{
		float PerspectiveFOV = 1.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 10000.0f;
	};

	struct CameraOrthographicProperties
	{
		float OrthographicSize = 1.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;
	};


	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projectionMatrix);
		virtual ~Camera() = default;

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};

}