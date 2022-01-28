#include "stdafx.h"
#include "Property.h"

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {


	template <>
	bool Property<void*>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<void*>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
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
		XYZ_PROFILE_FUNC("Property<uint32_t>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<uint32_t>& curr = m_Keys[current];
			const KeyFrame<uint32_t>& next = m_Keys[current + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

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
		XYZ_PROFILE_FUNC("Property<float>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<float>& curr = m_Keys[current];
			const KeyFrame<float>& next = m_Keys[current + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

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
		XYZ_PROFILE_FUNC("Property<glm::vec2>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec2>& curr = m_Keys[current];
			const KeyFrame<glm::vec2>& next = m_Keys[current + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec3>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec3>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec3>& curr = m_Keys[current];
			const KeyFrame<glm::vec3>& next = m_Keys[current + 1];

			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<glm::vec4>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec4>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<glm::vec4>& curr = m_Keys[current];
			const KeyFrame<glm::vec4>& next = m_Keys[current + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
			if (frame >= next.Frame)
				current++;
			return false;
		}
		return true;
	}

	bool Property<Ref<SubTexture>>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<SubTexture>>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
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

	bool Property<Ref<Material>>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<Material>>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			size_t& current = m_CurrentKey;
			const KeyFrame<Ref<Material>>& curr = m_Keys[current];
			if (frame >= curr.Frame)
			{
				*m_Value = m_Keys[current + 1].Value;
				current++;
			}
			return false;
		}
		return true;
	}

	bool Property<glm::mat4>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::mat4>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			return false;
		}
		return true;
	}
	bool Property<bool>::Update(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<bool>::Update");
		m_Value = m_GetPropertyReference(m_Entity);
		if (isKeyInRange() && frame <= Length() && m_Value != nullptr)
		{
			return false;
		}
		return true;
	}
}