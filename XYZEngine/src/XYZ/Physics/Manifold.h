#pragma once
#include "PhysicsBody.h"


namespace XYZ {

	struct Manifold
	{
		PhysicsBody* A;
		PhysicsBody* B;
		glm::vec2 Normal;
		float PenetrationDepth;
	};
}