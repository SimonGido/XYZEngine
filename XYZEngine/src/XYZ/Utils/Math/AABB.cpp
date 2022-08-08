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
		const glm::vec3 diff = Max - Min;
		return 2.0f * (diff.x * diff.y * diff.z + diff.z * diff.x);
	}

	float AABB::GetPerimeter() const
	{
		const float wx = Max.x - Min.x;
		const float wy = Max.y - Min.y;
		return 2.0f * (wx + wy);
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

	bool AABB::Intersect(const AABB& aabb) const
	{
		const float d1x = aabb.Min.x - Max.x;
		const float d1y = aabb.Min.y - Max.y;
		const float d2x = Min.x - aabb.Max.x;
		const float d2y = Min.y - aabb.Max.y;

		if (d1x > 0.0f || d1y > 0.0f)
			return false;

		if (d2x > 0.0f || d2y > 0.0f)
			return false;

		return true;
	}

	AABB AABB::Union(const AABB& a, const AABB& b)
	{
		AABB c;
		c.Min.x = std::min(a.Min.x, b.Min.x);
		c.Min.y = std::min(a.Min.y, b.Min.y);

		c.Max.x = std::max(a.Max.x, b.Max.x);
		c.Max.y = std::max(a.Max.y, b.Max.y);
		return c;
	}

	AABB AABB::operator+(const glm::vec2& val) const
	{
		AABB res;
		res.Min = Min + glm::vec3(val, 0.0f);
		res.Max = Max + glm::vec3(val, 0.0f);
		return res;
	}

}