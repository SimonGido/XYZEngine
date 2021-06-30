#include "stdafx.h"
#include "Track.h"

#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace XYZ {

	template <>
	bool Property<float>::Update(float& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<float>& curr = m_Keys[current];
			const KeyFrame<float>& next = m_Keys[current + 1];
			float length = next.EndTime - curr.EndTime;
			float passed = time - curr.EndTime;

			val = glm::lerp(curr.Value, next.Value, passed / length);
			if (time >= next.EndTime)
				current++;
			return false;
		}
		return true;
	}
	template <>
	bool Property<glm::vec2>::Update(glm::vec2& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<glm::vec2>& curr = m_Keys[current];
			const KeyFrame<glm::vec2>& next = m_Keys[current + 1];
			float length = next.EndTime - curr.EndTime;
			float passed = time - curr.EndTime;

			val = glm::lerp(curr.Value, next.Value, passed / length);
			if (time >= next.EndTime)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec3>::Update(glm::vec3& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<glm::vec3>& curr = m_Keys[current];
			const KeyFrame<glm::vec3>& next = m_Keys[current + 1];
			float length = next.EndTime - curr.EndTime;
			float passed = time - curr.EndTime;

			val = glm::lerp(curr.Value, next.Value, passed / length);
			if (time >= next.EndTime)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec4>::Update(glm::vec4& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<glm::vec4>& curr = m_Keys[current];
			const KeyFrame<glm::vec4>& next = m_Keys[current + 1];
			float length = next.EndTime - curr.EndTime;
			float passed = time - curr.EndTime;

			val = glm::lerp(curr.Value, next.Value, passed / length);
			if (time >= next.EndTime)
				current++;
			return false;
		}
		return true;
	}

	bool Property<Ref<SubTexture>>::Update(Ref<SubTexture>& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<Ref<SubTexture>>& curr = m_Keys[current];
			if (time >= curr.EndTime)
			{
				val = m_Keys[current + 1].Value;
				current++;
			}
			return false;
		}
		return true;
	}
}