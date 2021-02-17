#pragma once

#include "XYZ/Gui/CanvasRenderer.h"
#include "XYZ/Gui/RectTransform.h"
#include "XYZ/Gui/LineRenderer.h"
#include "XYZ/Renderer/Camera.h"
#include <glm/glm.hpp>


namespace XYZ {

	struct GuiRendererCamera
	{
		XYZ::Camera Camera;
		glm::mat4 ViewMatrix;
	};

	class GuiRenderer
	{
	public:
		static void Init();

		static void BeginScene(const GuiRendererCamera& camera, const glm::vec2& windowSize);
		static void EndScene();

		static void SubmitWidget(CanvasRenderer* canvasRenderer, RectTransform* transform);
		static void SubmitWidget(LineRenderer* lineRenderer);
	private:
		static void flushDrawList();
		static bool cullTest(const glm::vec3& position, const glm::vec2& scale);
	};
}