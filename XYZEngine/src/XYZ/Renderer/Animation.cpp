#include "stdafx.h"
#include "Animation.h"
#include "SubTexture.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

	
	template<>
	glm::vec3 Interpolate(const glm::vec3& start, const glm::vec3& end, float startTime, float endTime, float time)
	{
		float scale = (time - startTime) / (endTime - startTime);
		glm::vec3 valueDiff = end - start;
		return start + (valueDiff * scale);
	}

	template<>
	void Interpolate(TransformComponent& out, const TransformComponent& start, const TransformComponent& end, float startTime, float endTime, float time)
	{
		float scale = (time - startTime) / (endTime - startTime);

		glm::vec3 translationDiff = end.Translation - start.Translation;
		out.Translation = start.Translation + (translationDiff * scale);
	
		glm::vec3 rotationDiff = end.Rotation - start.Rotation;
		out.Rotation = start.Rotation + (rotationDiff * scale);

		glm::vec3 scaleDiff = end.Scale - start.Scale;
		out.Scale = start.Scale + (scaleDiff * scale);
	}

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
		if (m_CurrentTime >= m_Length)
		{
			if (!m_Repeat)
				return;
			m_CurrentTime = 0.0f;
		}
		for (auto& track : m_Tracks)
			track->Update(m_CurrentTime);

		m_CurrentTime += ts;
	}

	void Animation::UpdateLength()
	{
		for (auto& track : m_Tracks)
			m_Length = std::max(m_Length, track->Length());
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

	ITrack::ITrack(SceneEntity entity)
		:
		m_Entity(entity)
	{
	}
	ITrack::ITrack(const ITrack& other)
		:
		m_Entity(other.m_Entity)
	{
	}
}