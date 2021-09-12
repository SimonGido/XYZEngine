#include "stdafx.h"
#include "SpriteRendererTrack.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	SpriteRendererTrack::SpriteRendererTrack(SceneEntity entity)
		:
		Track(entity)
	{
		m_Type = TrackType::SpriteRenderer;
	}
	bool SpriteRendererTrack::Update(uint32_t frame)
	{
		SpriteRenderer& spriteRenderer = m_Entity.GetComponent<SpriteRenderer>();
		bool finished = m_SpriteProperty.Update(spriteRenderer.SubTexture, frame);
		finished &= m_ColorProperty.Update(spriteRenderer.Color, frame);
		return finished;
	}
	void SpriteRendererTrack::Reset()
	{
		m_SpriteProperty.Reset();
		m_ColorProperty.Reset();
	}
	uint32_t SpriteRendererTrack::Length() const
	{
		uint32_t length = m_SpriteProperty.Length();
		length = std::max(length, m_ColorProperty.Length());
		return length;
	}
	void SpriteRendererTrack::AddKeyFrame(const KeyFrame<Ref<SubTexture>>& spriteKey)
	{
		m_SpriteProperty.AddKeyFrame(spriteKey);
	}
	void SpriteRendererTrack::AddKeyFrame(const KeyFrame<glm::vec4>& colorKey)
	{
		m_ColorProperty.AddKeyFrame(colorKey);
	}
	void SpriteRendererTrack::RemoveSpriteKeyFrame(uint32_t frame)
	{
		m_SpriteProperty.RemoveKeyFrame(frame);
	}
	void SpriteRendererTrack::RemoveColorKeyFrame(uint32_t frame)
	{
		m_ColorProperty.RemoveKeyFrame(frame);
	}
	void SpriteRendererTrack::updatePropertyCurrentKey(uint32_t frame)
	{
		m_SpriteProperty.UpdateCurrentKey(frame);
		m_ColorProperty.UpdateCurrentKey(frame);
	}
}