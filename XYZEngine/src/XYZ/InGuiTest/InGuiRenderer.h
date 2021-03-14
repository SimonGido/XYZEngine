#pragma once

#include "InGuiElement.h"

namespace XYZ {

	struct IGQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
		uint32_t  ScissorIndex = 0;
	};

	struct IGLine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
	};

	struct IGMesh
	{
		std::vector<IGQuad>	Quads;
		std::vector<IGLine>	Lines;
	};

	class IGRenderer
	{
	public:
	
	private:
		IGMesh m_Mesh;
	};
}