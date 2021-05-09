#include "stdafx.h"
#include "Animation.h"
#include "SubTexture.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {



	//template <>
	//float Interpolate(const KeyFrame<float>& begin, const KeyFrame<float>& end, float time)
	//{
	//	float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
	//	float valueDiff = end.Value - begin.Value;
	//
	//	return begin.Value + (valueDiff * scale);
	//}
	
	template<>
	glm::vec3 Interpolate(const glm::vec3& start, const glm::vec3& end, float startTime, float endTime, float time)
	{
		float scale = (time - startTime) / (endTime - startTime);
		glm::vec3 valueDiff = end - start;
		return start + (valueDiff * scale);
	}

	//template<>
	//glm::vec4 Interpolate(const KeyFrame<glm::vec4>& begin, const KeyFrame<glm::vec4>& end, float time)
	//{
	//	float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
	//	glm::vec4 valueDiff = end.Value - begin.Value;
	//
	//	glm::vec4 result = { 0, 0, 0 ,0 };
	//	result.x = begin.Value.x + (valueDiff.x * scale);
	//	result.y = begin.Value.y + (valueDiff.y * scale);
	//	result.z = begin.Value.z + (valueDiff.z * scale);
	//	result.w = begin.Value.w + (valueDiff.w * scale);
	//
	//	return result;
	//}
	//
	//template<>
	//glm::mat4 Interpolate(const KeyFrame<glm::mat4>& begin, const KeyFrame<glm::mat4>& end, float time)
	//{
	//	// Scale does not work
	//	float scale = (time - begin.EndTime) / (end.EndTime - begin.EndTime);
	//	return glm::interpolate(begin.Value, end.Value, scale);
	//}
	//
	//
	//template<>
	//Ref<SubTexture> Interpolate(const KeyFrame<Ref<SubTexture>>& begin, const KeyFrame<Ref<SubTexture>>& end, float time)
	//{
	//	return begin.Value;
	//}
	//
	//template<>
	//Property<float>::Property()
	//	:
	//	IProperty(PropertyType::Float),
	//	m_Value(nullptr)
	//{
	//}
	//template<>
	//Property<float>::Property(const Property<float>& other)
	//	: 
	//	IProperty(other.m_Type),
	//	m_Value(other.m_Value),
	//	m_KeyFrames(other.m_KeyFrames)
	//{
	//}
	//template <>
	//void Property<float>::Update(float time)
	//{
	//	if (m_CurrentFrame < m_KeyFrames.size())
	//	{					
	//		*m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
	//		if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
	//			m_CurrentFrame++;
	//	}
	//}
	//
	//template<>
	//Property<glm::vec3>::Property()
	//	:
	//	IProperty(PropertyType::Vec3),
	//	m_Value(nullptr)
	//{
	//}
	//template<>
	//Property<glm::vec3>::Property(const Property<glm::vec3>& other)
	//	: 
	//	IProperty(other.m_Type),
	//	m_Value(other.m_Value),
	//	m_KeyFrames(other.m_KeyFrames)
	//{
	//}
	//template <>
	//void Property<glm::vec3>::Update(float time)
	//{
	//	if (m_CurrentFrame < m_KeyFrames.size())
	//	{					
	//		*m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
	//		if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
	//			m_CurrentFrame++;
	//	}
	//}
	//
	//template<>
	//Property<glm::vec4>::Property()
	//	:
	//	IProperty(PropertyType::Vec4),
	//	m_Value(nullptr)
	//{
	//}
	//
	//template<>
	//Property<glm::vec4>::Property(const Property<glm::vec4>& other)
	//	: 
	//	IProperty(other.m_Type),
	//	m_Value(other.m_Value),
	//	m_KeyFrames(other.m_KeyFrames)
	//{
	//}
	//template <>
	//void Property<glm::vec4>::Update(float time)
	//{
	//	if (m_CurrentFrame < m_KeyFrames.size())
	//	{					
	//		*m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
	//		if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
	//			m_CurrentFrame++;
	//	}
	//}
	//
	//template<>
	//Property<glm::mat4>::Property()
	//	:
	//	IProperty(PropertyType::Vec4),
	//	m_Value(nullptr)
	//{
	//}
	//
	//template<>
	//Property<glm::mat4>::Property(const Property<glm::mat4>& other)
	//	: 
	//	IProperty(other.m_Type),
	//	m_Value(other.m_Value),
	//	m_KeyFrames(other.m_KeyFrames)
	//{
	//}
	//template <>
	//void Property<glm::mat4>::Update(float time)
	//{
	//	if (m_CurrentFrame < m_KeyFrames.size())
	//	{					
	//		*m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
	//		if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
	//			m_CurrentFrame++;
	//	}
	//}
	//
	//template<>
	//Property<Ref<SubTexture>>::Property()
	//	:
	//	IProperty(PropertyType::Sprite),
	//	m_Value(nullptr)
	//{
	//}
	//template<>
	//Property<Ref<SubTexture>>::Property(const Property<Ref<SubTexture>>& other)
	//	: 
	//	IProperty(other.m_Type),
	//	m_Value(other.m_Value),
	//	m_KeyFrames(other.m_KeyFrames)
	//{
	//}
	//
	//void Property <Ref<SubTexture>>::Update(float time)
	//{
	//	if (m_CurrentFrame < m_KeyFrames.size())
	//	{
	//		*m_Value = interpolate(m_KeyFrames[m_CurrentFrame], m_KeyFrames[m_CurrentFrame + 1], time);
	//		if (time >= m_KeyFrames[m_CurrentFrame].EndTime)
	//			m_CurrentFrame++;
	//	}
	//}
	
	Animation::Animation(SceneEntity entity)
		:
		m_Entity(entity),
		m_Length(0.0f),
		m_CurrentTime(0.0f),
		m_Repeat(true)
	{
	}

	void Animation::Update(Timestep ts)
	{
		if (m_CurrentTime < m_Length)
		{
			for (size_t i = 0; i < m_Tracks.Size(); ++i)
				m_Tracks.Get<Track>(i).Update(m_CurrentTime);

			m_CurrentTime += ts;
		}
		else if (m_Repeat)
		{
			m_CurrentTime = 0.0f;
			for (size_t i = 0; i < m_Tracks.Size(); ++i)
				m_Tracks.Get<Track>(i).Reset();
		}
	}

	void Animation::UpdateLength()
	{
		for (size_t i = 0; i < m_Tracks.Size(); ++i)
			m_Length = std::max(m_Length, m_Tracks.Get<Track>(i).Length());
	}
	

	bool Property<glm::vec3>::Update(glm::vec3& val, float time)
	{
		size_t& current = m_CurrentFrame;		
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<glm::vec3>& key = m_Keys[current];
			const KeyFrame<glm::vec3>& next = m_Keys[current + 1];
			
			val = Interpolate<glm::vec3>(key.Value, next.Value, key.EndTime, next.EndTime, time);
			if (time >= next.EndTime)
				current++;

			return false;
		}
		return true;
	}

	Track::Track(SceneEntity entity)
		:
		m_Entity(entity)
	{
	}
	Track::Track(const Track& other)
		:
		m_Entity(other.m_Entity)
	{
	}

	TransformTrack::TransformTrack(SceneEntity entity)
		: 
		Track(entity)
	{
	}

	TransformTrack::TransformTrack(const TransformTrack& other)
		:
		Track(other),
		m_TranslationProperty(other.m_TranslationProperty),
		m_RotationProperty(other.m_RotationProperty),
		m_ScaleProperty(other.m_ScaleProperty)
	{
	}

	TransformTrack::TransformTrack(TransformTrack&& other) noexcept
		:
		Track(std::move(other)),
		m_TranslationProperty(std::move(other.m_TranslationProperty)),
		m_RotationProperty(std::move(other.m_RotationProperty)),
		m_ScaleProperty(std::move(other.m_ScaleProperty))
	{
	}
	void TransformTrack::OnCopy(uint8_t* buffer)
	{
		new(buffer)TransformTrack(*this);
	}

	bool TransformTrack::Update(float time)
	{
		TransformComponent& transform = m_Entity.GetComponent<TransformComponent>();
		bool finished = m_TranslationProperty.Update(transform.Translation, time);
		finished = m_RotationProperty.Update(transform.Rotation, time);
		finished = m_ScaleProperty.Update(transform.Scale, time);
		return finished;
	}

	void TransformTrack::Reset()
	{
		m_TranslationProperty.Reset();
		m_RotationProperty.Reset();
		m_ScaleProperty.Reset();
	}

	float TransformTrack::Length()
	{
		float length = m_TranslationProperty.Length();
		length = std::max(length, m_RotationProperty.Length());
		length = std::max(length, m_ScaleProperty.Length());
		return length;
	}

	template <>
	void TransformTrack::AddKeyFrame<glm::vec3, TransformTrack::PropertyType::Translation>(const KeyFrame<glm::vec3>& key)
	{
		m_TranslationProperty.AddKeyFrame(key);
	}
	template <>
	void TransformTrack::AddKeyFrame<glm::vec3, TransformTrack::PropertyType::Rotation>(const KeyFrame<glm::vec3>& key)
	{
		m_RotationProperty.AddKeyFrame(key);
	}
	template <>
	void TransformTrack::AddKeyFrame<glm::vec3, TransformTrack::PropertyType::Scale>(const KeyFrame<glm::vec3>& key)
	{
		m_ScaleProperty.AddKeyFrame(key);
	}
}