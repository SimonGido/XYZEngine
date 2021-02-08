#pragma once

#include "XYZ/Utils/Math/AABB.h"
#include <glm/glm.hpp>

namespace XYZ {

	struct PhysicsShape
	{
		virtual AABB GetAABB() const = 0;
	};

	struct BoxShape : public PhysicsShape
	{
		glm::vec2 Min;
		glm::vec2 Max;           

		virtual AABB GetAABB() const { return { Min, Max }; }
	};
}
