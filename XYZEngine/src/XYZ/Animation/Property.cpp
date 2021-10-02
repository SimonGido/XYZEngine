#include "stdafx.h"
#include "Property.h"

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace XYZ {

	template<>
	void* Property<void*>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		return m_Keys[current].Value;
	}
	template<>
	bool Property<bool>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		return m_Keys[current].Value;
	}
	template<>
	uint32_t Property<uint32_t>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return currKey.Value + (nextKey.Value - currKey.Value) * (float)passed / (float)length;
	}

	template<>
	float Property<float>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return glm::lerp(currKey.Value, nextKey.Value, (float)passed / (float)length);
	}

	template<>
	glm::vec2 Property<glm::vec2>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return glm::lerp(currKey.Value, nextKey.Value, (float)passed / (float)length);
	}

	template<>
	glm::vec3 Property<glm::vec3>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next    = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return glm::lerp(currKey.Value, nextKey.Value, (float)passed / (float)length);
	}

	template<>
	glm::vec4 Property<glm::vec4>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return glm::lerp(currKey.Value, nextKey.Value, (float)passed / (float)length);
	}
	template<>
	glm::mat4 Property<glm::mat4>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		size_t next = current + 1;
		const auto& currKey = m_Keys[current];
		if (next >= m_Keys.size() || currKey.Frame == frame)
			return currKey.Value;

		const auto& nextKey = m_Keys[current + 1];
		uint32_t length = nextKey.Frame - currKey.Frame;
		uint32_t passed = frame - currKey.Frame;

		return currKey.Value;
	}
	template<>
	Ref<SubTexture> Property<Ref<SubTexture>>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		const KeyFrame<Ref<SubTexture>>& next = m_Keys[current + 1];
		return next.Value;
	}

	template<>
	Ref<Material> Property<Ref<Material>>::GetValue(uint32_t frame) const
	{
		size_t current = FindKey(frame);
		const KeyFrame<Ref<Material>>& next = m_Keys[current + 1];
		return next.Value;
	}

	template <>
	bool Property<void*>::Update(uint32_t frame)
	{
		if (isKeyInRange() && frame <= Length())
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<void*>& curr = m_Keys[current];
			const KeyFrame<void*>& next = m_Keys[current + 1];

			*m_Value = curr.Value;
			if (frame >= next.Frame)
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
			uint32_t length = next.Frame - curr.Frame;
			uint32_t passed = frame - curr.Frame;

			*m_Value = curr.Value + (next.Value - curr.Value) * (float)passed / (float)length;
			if (frame >= next.Frame)
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
			uint32_t length = next.Frame - curr.Frame;
			uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
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
			uint32_t length = next.Frame - curr.Frame;
			uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
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

			uint32_t length = next.Frame - curr.Frame;
			uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
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
			uint32_t length = next.Frame - curr.Frame;
			uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
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
			if (frame >= curr.Frame)
			{
				*m_Value = m_Keys[current + 1].Value;
				current++;
			}
			return false;
		}
		return true;
	}
}