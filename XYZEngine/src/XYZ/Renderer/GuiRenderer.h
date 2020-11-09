#pragma once

#include "XYZ/Gui/CanvasRenderer.h"
#include "XYZ/Gui/RectTransform.h"

#include <glm/glm.hpp>


namespace XYZ {

	class GuiRenderer
	{
	public:
		static void Init();

		static void BeginScene(const glm::vec2& viewportSize);
		static void EndScene();

		static void SetMaterial(const Ref<Material>& material);
		static void SubmitWidget(CanvasRenderer* canvasRenderer, RectTransform* transform);

	private:
		static void flushDrawList();
		static bool cullTest(const glm::vec3& position, const glm::vec2& scale);
	};
}