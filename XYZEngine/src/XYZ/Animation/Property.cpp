#include "stdafx.h"
#include "Property.h"

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Asset/MaterialAsset.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {


	template <>
	void Property<void*>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<void*>::Update");

		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<void*>& curr = Keys[key];
			const KeyFrame<void*>& next = Keys[key + 1];

			*m_Value = curr.Value;
		}
	}

	template <>
	void Property<uint32_t>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<uint32_t>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<uint32_t>& curr = Keys[key];
			const KeyFrame<uint32_t>& next = Keys[key + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = curr.Value + (next.Value - curr.Value) * (float)passed / (float)length;
		}
	}
	template <>
	void Property<float>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<float>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<float>& curr = Keys[key];
			const KeyFrame<float>& next = Keys[key + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
		}
	}

	template <>
	void Property<glm::vec2>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec2>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<glm::vec2>& curr = Keys[key];
			const KeyFrame<glm::vec2>& next = Keys[key + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
		}
	}

	void Property<glm::vec3>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec3>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<glm::vec3>& curr = Keys[key];
			const KeyFrame<glm::vec3>& next = Keys[key + 1];

			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
		}
	}

	void Property<glm::vec4>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec4>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<glm::vec4>& curr = Keys[key];
			const KeyFrame<glm::vec4>& next = Keys[key + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
		}
	}
	void Property<glm::quat>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::vec4>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			const KeyFrame<glm::quat>& curr = Keys[key];
			const KeyFrame<glm::quat>& next = Keys[key + 1];
			const uint32_t length = next.Frame - curr.Frame;
			const uint32_t passed = frame - curr.Frame;

			*m_Value = glm::lerp(curr.Value, next.Value, (float)passed / (float)length);
		}
	}
	void Property<Ref<SubTexture>>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<SubTexture>>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			*m_Value = Keys[key].Value;
		}
	}

	void Property<Ref<MaterialAsset>>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<MaterialAsset>>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			*m_Value = Keys[key].Value;
		}
	}
	void Property<Ref<Material>>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<Material>>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			*m_Value = Keys[key].Value;
		}
	}
	void Property<Ref<MaterialInstance>>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<Ref<MaterialInstance>>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
			*m_Value = Keys[key].Value;
		}
	}
	void Property<glm::mat4>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<glm::mat4>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
		}
	}
	void Property<bool>::Update(size_t key, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("Property<bool>::Update");
		XYZ_ASSERT(std::is_sorted(Keys.begin(), Keys.end()), "Key frames are not sorted");
		XYZ_ASSERT(key < static_cast<int64_t>(Keys.size()) - 1, "Key indices are out of range");

		m_Value = m_GetPropertyReference(m_Entity);
		if (m_Value != nullptr)
		{
		}
	}
}