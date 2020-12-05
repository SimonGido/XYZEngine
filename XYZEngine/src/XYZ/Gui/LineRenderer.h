#pragma once

#include "XYZ/ECS/Component.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct LineRenderer : public Type<LineRenderer>
	{
		LineRenderer(const glm::vec4& color, const LineMesh& mesh, bool isVisible = false);


		glm::vec4 Color;
		LineMesh  LineMesh;
		bool	  IsVisible = true;
	};
	
}