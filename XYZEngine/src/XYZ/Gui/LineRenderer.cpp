#include "stdafx.h"
#include "LineRenderer.h"


namespace XYZ {
	LineRenderer::LineRenderer(const glm::vec4& color, const XYZ::LineMesh& mesh, bool isVisible)
		: Color(color), LineMesh(mesh), IsVisible(isVisible)
	{
	}
}