#pragma once
#include <glm/glm.hpp>

namespace XYZ {

	struct Player
	{
		Player()
			: Position(0.0f), Size(1.0f), Color(1.0f), ID(0)
		{}
		glm::vec2 Position;
		glm::vec2 Size;
		glm::vec4 Color;
		uint32_t  ID;
	};

}