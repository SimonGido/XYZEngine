#pragma once
#include "XYZ/Core/Ref.h"

#include <glm/glm.hpp>


namespace XYZ {

	struct Vertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
	};

	struct Mesh : public RefCount
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;
		int32_t TextureID;
	};
}
