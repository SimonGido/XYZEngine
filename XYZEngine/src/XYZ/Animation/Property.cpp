#include "stdafx.h"
#include "Property.h"

#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace XYZ {


	template <>
	bool Property<void*>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<void*>& curr = m_Keys[current];
			const KeyFrame<void*>& next = m_Keys[current + 1];

			*m_Value = curr.Value;
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	template <>
	bool Property<uint32_t>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<uint32_t>& curr = m_Keys[current];
			const KeyFrame<uint32_t>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			*m_Value = curr.Value + (next.Value - curr.Value) * (float)passed / (float)length;
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}
	template <>
	bool Property<float>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<float>& curr = m_Keys[current];
			const KeyFrame<float>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	template <>
	bool Property<glm::vec2>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec2>& curr = m_Keys[current];
			const KeyFrame<glm::vec2>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec3>::Update(uint32_t frame)
	{	
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec3>& curr = m_Keys[current];
			const KeyFrame<glm::vec3>& next = m_Keys[current + 1];

			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec4>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec4>& curr = m_Keys[current];
			const KeyFrame<glm::vec4>& next = m_Keys[current + 1];
			uint32_t length = next.EndFrame - curr.EndFrame;
			uint32_t passed = frame - curr.EndFrame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.EndFrame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<Ref<SubTexture>>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<Ref<SubTexture>>& curr = m_Keys[current];
			if (frame >= curr.EndFrame)
			{
				*m_Value = m_Keys[current + 1].Value;
				current++;
			}
			return false;
		}
		return true;
	}
}