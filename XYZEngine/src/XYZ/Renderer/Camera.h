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

		void SetPerspective(const CameraPerspectiveProperties& perspectiveProps);
		void SetOrthographic(const CameraOrthographicProperties& orthoProps);
		void SetProjectionType(CameraProjectionType type) { m_ProjectionType = type; }
		void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		const CameraPerspectiveProperties& GetPerspectiveProperties() const { return m_PerspectiveProperties; }
		const CameraOrthographicProperties& GetOrthographicProperties() const { return m_OrthographicProperties; }
		const CameraProjectionType GetProjectionType() const { return m_ProjectionType; }
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

		CameraProjectionType m_ProjectionType = CameraProjectionType::Orthographic;

		CameraPerspectiveProperties m_PerspectiveProperties;
		CameraOrthographicProperties m_OrthographicProperties;
	};
}