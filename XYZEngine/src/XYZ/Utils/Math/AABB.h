#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Utils/Math/Math.h"

#include <glm/glm.hpp>
#include <algorithm>

namespace XYZ {

	struct XYZ_API AABB
	{
		glm::vec3 Min, Max;

		AABB();
		AABB(const glm::vec3& min, const glm::vec3& max);
			
		float GetPerimeter() const;
		float CalculateArea() const;
		bool Contains(const AABB& aabb) const;
		bool Intersect(const AABB& aabb) const;

		bool InsideFrustum(const glm::mat4& mvp) const;
		bool IsOnPlane(const Math::Plane& plane) const;
		bool InsideFrustum(const Math::Frustum& frustum) const;

		glm::vec3 GetCenter() const;


		AABB TransformAABB(const glm::mat4& transform) const;

		static AABB Union(const AABB& a, const AABB& b);

		AABB operator +(const glm::vec2& val) const;
	};
}