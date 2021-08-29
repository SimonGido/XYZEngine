#include "stdafx.h"
#include "TransformTrack.h"

#include "XYZ/Scene/Components.h"

#include <glm/glm.hpp>


namespace XYZ {
	TransformTrack::TransformTrack(SceneEntity entity)
		: Track(entity)
	{
		m_Type = TrackType::Transform;
	}

	bool TransformTrack::Update(float time)
	{
		TransformComponent& transform = m_Entity.GetComponent<TransformComponent>();
		bool finished = m_TranslationProperty.Update(transform.Translation, time);
		finished	 &= m_RotationProperty.Update(transform.Rotation, time);
		finished	 &= m_ScaleProperty.Update(transform.Scale, time);
		return finished;
	}

	void TransformTrack::Reset()
	{
		m_TranslationProperty.Reset();
		m_RotationProperty.Reset();
		m_ScaleProperty.Reset();
	}

	float TransformTrack::Length() const
	{
		float length = m_TranslationProperty.Length();
		length = std::max(length, m_RotationProperty.Length());
		length = std::max(length, m_ScaleProperty.Length());
		return length;
	}


	void TransformTrack::AddKeyFrame(const KeyFrame<glm::vec3>& key, PropertyType type)
	{
		switch (type)
		{
		case XYZ::TransformTrack::PropertyType::Translation:
			m_TranslationProperty.AddKeyFrame(key);
			break;
		case XYZ::TransformTrack::PropertyType::Rotation:
			m_RotationProperty.AddKeyFrame(key);
			break;
		case XYZ::TransformTrack::PropertyType::Scale:
			m_ScaleProperty.AddKeyFrame(key);
			break;
		default:
			break;
		}
	}
}