#pragma once
#include "XYZ/Core/Ref.h"

#include <glm/glm.hpp>


namespace XYZ {

	struct Vertex
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct Mesh
	{
		std::vector<Vertex> Vertices;
	};
}
