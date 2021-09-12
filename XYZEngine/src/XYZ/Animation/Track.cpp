#include "stdafx.h"
#include "Track.h"

#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace XYZ {
	Track::Track(SceneEntity entity)
		: m_Entity(entity)
	{}

	template <>
	bool Property<float>::Update(float& val, uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<float>& curr = m_Keys[current];
			const KeyFrame<float>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			val = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}
	template <>
	bool Property<glm::vec2>::Update(glm::vec2& val, uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec2>& curr = m_Keys[current];
			const KeyFrame<glm::vec2>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			val = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec3>::Update(glm::vec3& val, uint32_t frame)
	{	
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec3>& curr = m_Keys[current];
			const KeyFrame<glm::vec3>& next = m_Keys[current + 1];

			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			val = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec4>::Update(glm::vec4& val, uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec4>& curr = m_Keys[current];
			const KeyFrame<glm::vec4>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			val = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<Ref<SubTexture>>::Update(Ref<SubTexture>& val, uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<Ref<SubTexture>>& curr = m_Keys[current];
			if (frame >= curr.EndFrame)
			{
				val = m_Keys[current + 1].Value;
				current++;
			}
			return false;
		}
		return true;
	}
}