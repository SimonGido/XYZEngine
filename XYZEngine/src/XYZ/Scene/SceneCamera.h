#pragma once
#include "XYZ/Renderer/Camera.h"


namespace XYZ {

	struct SceneCameraPerspectiveProperties
	{
		float PerspectiveFOV = 1.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 10000.0f;
	};

	struct SceneCameraOrthographicProperties
	{
		float OrthographicSize = 10.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;
	};

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera();

		void SetPerspective(const SceneCameraPerspectiveProperties& perspectiveProps);
		void SetOrthographic(const SceneCameraOrthographicProperties& orthoProps);
		void SetProjectionType(CameraProjectionType type) { m_ProjectionType = type; }

		void SetViewportSize(uint32_t width, uint32_t height);

		const SceneCameraPerspectiveProperties& GetPerspectiveProperties() const { return m_PerspectiveProperties; }
		const SceneCameraOrthographicProperties& GetOrthographicProperties() const { return m_OrthographicProperties; }

		CameraProjectionType GetProjectionType() const { return m_ProjectionType; }
	private:
		CameraProjectionType m_ProjectionType = CameraProjectionType::Orthographic;

		SceneCameraPerspectiveProperties m_PerspectiveProperties;
		SceneCameraOrthographicProperties m_OrthographicProperties;
	};
}