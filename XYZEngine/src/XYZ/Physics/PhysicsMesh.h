#pragma once
#include <box2d/box2d.h>

#include <vector>

namespace XYZ {
	struct PhysicsMesh
	{
		std::vector<b2Vec2> Vertices;
	};
}