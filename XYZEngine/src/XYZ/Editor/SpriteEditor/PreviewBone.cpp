#include "stdafx.h"
#include "PreviewBone.h"

#include "XYZ/Utils/Math/Math.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
	namespace Editor {
		void PreviewBone::Decompose(glm::vec2& start, glm::vec2& end, float& rot, glm::vec2& normal, bool worldTransform) const
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			if (worldTransform)
				glm::decompose(WorldTransform, scale, rotation, translation, skew, perspective);
			else
				glm::decompose(LocalTransform, scale, rotation, translation, skew, perspective);

			glm::vec3 euler = glm::eulerAngles(rotation);
			rot = euler.z;

			glm::vec2 tmpEnd = start + (Direction * Length);
			start = glm::vec2(translation.x, translation.y);
			glm::vec4 rotatedEnd = glm::toMat4(rotation) * glm::vec4(tmpEnd, 0.0f, 1.0f);
			end = start + glm::vec2(rotatedEnd.x, rotatedEnd.y);
			glm::vec2 dir = glm::normalize(end - start);
			normal = { -dir.y, dir.x };
		}
		bool PreviewBone::Collide(const glm::vec2& position, uint8_t collisionFlags) const
		{
			float rot;
			glm::vec2 start, end, normal;
			Decompose(start, end, rot, normal);
			if (IS_SET(collisionFlags, Start))
			{
				if (glm::distance(position, start) > PointRadius * 2.0f)
					return true;
			}
			if (IS_SET(collisionFlags, End))
			{
				if (glm::distance(position, end) > PointRadius * 2.0f)
					return true;
			}
			if (IS_SET(collisionFlags, Body))
			{
				glm::vec2 v1 = start + (normal * PointRadius);
				glm::vec2 v2 = start - (normal * PointRadius);
				if (Math::PointInTriangle(position, v1, v2, end))
					return true;
			}
			return false;
		}
		void PreviewBone::Rotate(const glm::vec2& position)
		{
			float rot;
			glm::vec2 start, end, normal;
			Decompose(start, end, rot, normal);
			glm::vec2 origDir = glm::normalize(end - start);
			glm::vec2 dir = glm::normalize(position - start);
			if (glm::distance(origDir, dir) > FLT_MIN)
			{
				float angle = glm::atan(dir.y, dir.x) - glm::atan(origDir.y, origDir.x);
				LocalTransform = glm::rotate(LocalTransform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}
		void PreviewBone::Translate(const glm::vec2& position, const glm::mat4& parentSpace)
		{
			float rot;
			glm::vec2 start, end, normal;
			Decompose(start, end, rot, normal);
			glm::mat4 translation = glm::inverse(parentSpace) * glm::translate(glm::vec3(position, 0.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0f, 0.0f, 1.0f));
			LocalTransform = translation * rotation;
		}
	}
}