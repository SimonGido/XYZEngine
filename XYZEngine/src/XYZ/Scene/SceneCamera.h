#pragma once
#include "XYZ/Renderer/Camera.h"


namespace XYZ {

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera();


		void SetViewportSize(uint32_t width, uint32_t height);

	private:
		void recalculate();

	private:
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
	};
}