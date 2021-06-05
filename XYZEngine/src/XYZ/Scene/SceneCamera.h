#pragma once
#include "XYZ/Renderer/Camera.h"


namespace XYZ {

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera();

		void SetProjectionType(CameraProjectionType type);
		void SetPerspective(const CameraPerspectiveProperties& props);
		void SetOrthographic(const CameraOrthographicProperties& props);

		void SetViewportSize(uint32_t width, uint32_t height);

		const CameraOrthographicProperties& GetOrthographicProperties() const { return m_OrthographicProps; }
		const CameraPerspectiveProperties& GetPerspectiveProperties() const { return m_PerspectiveProps; }
		CameraProjectionType GetProjectionType() const { return m_ProjectionType; }
	
		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }
	private:
		void recalculate();

	private:
		// It is set to 1 to prevent crash on recalculate perspective projection
		uint32_t m_ViewportWidth = 1;
		uint32_t m_ViewportHeight = 1;

		CameraProjectionType m_ProjectionType = CameraProjectionType::Orthographic;

		CameraPerspectiveProperties m_PerspectiveProps;
		CameraOrthographicProperties m_OrthographicProps;
	};
}