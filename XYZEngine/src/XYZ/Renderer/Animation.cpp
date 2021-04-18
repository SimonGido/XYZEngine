#include "stdafx.h"
#include "Animation.h"
#include "SubTexture.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

	IProperty::IProperty(PropertyType type)
		:
		m_Type(type)
	{
	}

	template<>
	float Property<float>::interpolate(const KeyFrame& begin, const KeyFrame& end, float time)
	{
		float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
		float valueDiff = end.Value - begin.Value;

		return begin.Value + (valueDiff * scale);
	}

	template<>
	glm::vec3 Property<glm::vec3>::interpolate(const KeyFrame& begin, const KeyFrame& end, float time)
	{
		float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
		glm::vec3 valueDiff = end.Value - begin.Value;

		glm::vec3 result = { 0,0,0 };
		result.x = begin.Value.x + (valueDiff.x * scale);
		result.y = begin.Value.y + (valueDiff.y * scale);
		result.z = begin.Value.z + (valueDiff.z * scale);

		return result;
	}

	template<>
	glm::vec4 Property<glm::vec4>::interpolate(const KeyFrame& begin, const KeyFrame& end, float time)
	{
		float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
		glm::vec4 valueDiff = end.Value - begin.Value;

		glm::vec4 result = { 0, 0, 0 ,0 };
		result.x = begin.Value.x + (valueDiff.x * scale);
		result.y = begin.Value.y + (valueDiff.y * scale);
		result.z = begin.Value.z + (valueDiff.z * scale);
		result.w = begin.Value.w + (valueDiff.w * scale);

		return result;
	}

	template<>
	glm::mat4 Property<glm::mat4>::interpolate(const KeyFrame& begin, const KeyFrame& end, float time)
	{
		// Scale does not work
		float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
		return glm::interpolate(begin.Value, end.Value, scale);
	}


	template<>
	Ref<SubTexture> Property<Ref<SubTexture>>::interpolate(const KeyFrame& begin, const KeyFrame& end, float time)
	{
		return begin.Value;
	}

	template<>
	Property<float>::Property(float& value)
		:
		IProperty(PropertyType::Float),
		m_Value(value)
	{
	}
	template<>
	Property<float>::Property(const Property<float>& other)
		: 
		IProperty(other.m_Type),
		m_Value(other.m_Value),
		m_KeyFrames(other.m_KeyFrames)
	{
	}
	template <>
	void Property<float>::Update(float time)
	{
		if (m_CurrentFrame < m_KeyFrames.size())
		{					
			m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
			if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
				m_CurrentFrame++;
		}
	}
	
	template<>
	Property<glm::vec3>::Property(glm::vec3& value)
		:
		IProperty(PropertyType::Vec3),
		m_Value(value)
	{
	}
	template<>
	Property<glm::vec3>::Property(const Property<glm::vec3>& other)
		: 
		IProperty(other.m_Type),
		m_Value(other.m_Value),
		m_KeyFrames(other.m_KeyFrames)
	{
	}
	template <>
	void Property<glm::vec3>::Update(float time)
	{
		if (m_CurrentFrame < m_KeyFrames.size())
		{					
			m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
			if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
				m_CurrentFrame++;
		}
	}

	template<>
	Property<glm::vec4>::Property(glm::vec4& value)
		:
		IProperty(PropertyType::Vec4),
		m_Value(value)
	{
	}

	template<>
	Property<glm::vec4>::Property(const Property<glm::vec4>& other)
		: 
		IProperty(other.m_Type),
		m_Value(other.m_Value),
		m_KeyFrames(other.m_KeyFrames)
	{
	}
	template <>
	void Property<glm::vec4>::Update(float time)
	{
		if (m_CurrentFrame < m_KeyFrames.size())
		{					
			m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
			if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
				m_CurrentFrame++;
		}
	}

	template<>
	Property<glm::mat4>::Property(glm::mat4& value)
		:
		IProperty(PropertyType::Vec4),
		m_Value(value)
	{
	}

	template<>
	Property<glm::mat4>::Property(const Property<glm::mat4>& other)
		: 
		IProperty(other.m_Type),
		m_Value(other.m_Value),
		m_KeyFrames(other.m_KeyFrames)
	{
	}
	template <>
	void Property<glm::mat4>::Update(float time)
	{
		if (m_CurrentFrame < m_KeyFrames.size())
		{					
			m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
			if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
				m_CurrentFrame++;
		}
	}

	template<>
	Property<Ref<SubTexture>>::Property(Ref<SubTexture>& value)
		:
		IProperty(PropertyType::Sprite),
		m_Value(value)
	{
	}
	template<>
	Property<Ref<SubTexture>>::Property(const Property<Ref<SubTexture>>& other)
		: 
		IProperty(other.m_Type),
		m_Value(other.m_Value),
		m_KeyFrames(other.m_KeyFrames)
	{
	}

	void Property <Ref<SubTexture>>::Update(float time)
	{
		if (m_CurrentFrame < m_KeyFrames.size())
		{
			m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
			if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
				m_CurrentFrame++;
		}
	}
	
	void Animation::Update(float dt)
	{
		if (m_CurrentTime < m_Length)
		{
			for (size_t i = 0; i < m_Properties.Size(); ++i)
				static_cast<IProperty*>(&m_Properties[i])->Update(m_CurrentTime);
			m_CurrentTime += dt;
		}
		else if (m_Repeat)
			m_CurrentTime = 0.0f;
	}
	void Animation::SetLength(float length)
	{
		m_Length = length;
	}
	void Animation::updateLength()
	{
		for (size_t i = 0; i < m_Properties.Size(); ++i)
			m_Length = std::max(m_Length, static_cast<IProperty*>(&m_Properties[i])->GetLength());
	}
}