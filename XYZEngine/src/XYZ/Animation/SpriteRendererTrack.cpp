#include "stdafx.h"
#include "SpriteRendererTrack.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	SpriteRendererTrack::SpriteRendererTrack(SceneEntity entity)
		:
		Track(entity)
	{
	}
	bool SpriteRendererTrack::Update(float time)
	{
		SpriteRenderer& spriteRenderer = m_Entity.GetComponent<SpriteRenderer>();
		bool finished = m_SpriteProperty.Update(spriteRenderer.SubTexture, time);
		finished &= m_ColorProperty.Update(spriteRenderer.Color, time);
		return finished;
	}
	void SpriteRendererTrack::Reset()
	{
		m_SpriteProperty.Reset();
		m_ColorProperty.Reset();
	}
	float SpriteRendererTrack::Length()
	{
		float length = m_SpriteProperty.Length();
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
}