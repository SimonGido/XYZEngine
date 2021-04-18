#pragma once
#include "Manifold.h"

namespace XYZ {
	namespace Collision {
		bool AABBvsAABB(Manifold& m);
		bool CirclevsCircle(Manifold& m);
		bool CirclevsAABB(Manifold& m);
		bool PolygonvsPolygon(Manifold& m);
	}
}