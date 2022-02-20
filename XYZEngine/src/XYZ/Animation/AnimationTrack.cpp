#include "stdafx.h"
#include "AnimationTrack.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>


namespace XYZ {
	void AnimationTrack<glm::vec4>::Update(size_t key, uint32_t frame, glm::vec4& value)
	{
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		const KeyFrame<glm::vec4>& curr = Keys[key];
		const KeyFrame<glm::vec4>& next = Keys[key + 1];

		const uint32_t length = next.Frame - curr.Frame;
		const uint32_t passed = frame - curr.Frame;

		value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
	}
	void AnimationTrack<glm::vec3>::Update(size_t key, uint32_t frame, glm::vec3& value)
	{
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");
		
		const KeyFrame<glm::vec3>& curr = Keys[key];
		const KeyFrame<glm::vec3>& next = Keys[key + 1];

		const uint32_t length = next.Frame - curr.Frame;
		const uint32_t passed = frame - curr.Frame;

		value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
	}
	void AnimationTrack<glm::vec2>::Update(size_t key, uint32_t frame, glm::vec2& value)
	{
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		const KeyFrame<glm::vec2>& curr = Keys[key];
		const KeyFrame<glm::vec2>& next = Keys[key + 1];

		const uint32_t length = next.Frame - curr.Frame;
		const uint32_t passed = frame - curr.Frame;

		value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
	}
}