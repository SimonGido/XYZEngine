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

	bool TransformTrack::Update(uint32_t frame)
	{
		TransformComponent& transform = m_Entity.GetComponent<TransformComponent>();
		bool finished = m_TranslationProperty.Update(transform.Translation, frame);
		finished	 &= m_RotationProperty.Update(transform.Rotation, frame);
		finished	 &= m_ScaleProperty.Update(transform.Scale, frame);
		return finished;
	}

	void TransformTrack::Reset()
	{
		m_TranslationProperty.Reset();
		m_RotationProperty.Reset();
		m_ScaleProperty.Reset();
	}

	uint32_t TransformTrack::Length() const
	{
		uint32_t length = m_TranslationProperty.Length();
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
	void TransformTrack::RemoveKeyFrame(uint32_t frame, PropertyType type)
	{
		switch (type)
		{
		case XYZ::TransformTrack::PropertyType::Translation:
			m_TranslationProperty.RemoveKeyFrame(frame);
			break;
		case XYZ::TransformTrack::PropertyType::Rotation:
			m_RotationProperty.RemoveKeyFrame(frame);
			break;
		case XYZ::TransformTrack::PropertyType::Scale:
			m_ScaleProperty.RemoveKeyFrame(frame);
			break;
		default:
			break;
		}
	}
	void TransformTrack::updatePropertyCurrentKey(uint32_t frame)
	{
		m_TranslationProperty.UpdateCurrentKey(frame);
		m_RotationProperty.UpdateCurrentKey(frame);
		m_ScaleProperty.UpdateCurrentKey(frame);
	}
}