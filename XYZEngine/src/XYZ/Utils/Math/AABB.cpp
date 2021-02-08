#include "stdafx.h"
#include "AABB.h"

namespace XYZ {

	AABB::AABB()
		: Min(0.0f), Max(0.0f) 
	{}
	AABB::AABB(const glm::vec3& min, const glm::vec3& max)
		: Min(min), Max(max) 
	{}
	float AABB::CalculateArea() const
	{
		glm::vec3 diff = Max - Min;
		return 2.0f * (diff.x * diff.y * diff.z + diff.z * diff.x);
	}

	bool AABB::Contains(const AABB& aabb) const
	{
		bool result = true;
		result = result && Min.x <= aabb.Min.x;
		result = result && Min.y <= aabb.Min.y;
		result = result && aabb.Max.x <=Max.x;
		result = result && aabb.Max.y <=Max.y;
		return result;
	}

	AABB AABB::Union(const AABB& a, const AABB& b)
	{
		AABB c;
		c.Min.x = std::min(a.Min.x, b.Min.x);
		c.Min.y = std::min(a.Min.y, b.Min.y);

		c.Max.x = std::min(a.Max.x, b.Max.x);
		c.Max.y = std::min(a.Max.y, b.Max.y);
		return c;
	}

}