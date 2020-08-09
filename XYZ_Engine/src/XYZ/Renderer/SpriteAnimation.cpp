#include "stdafx.h"
#include "SpriteAnimation.h"


namespace XYZ {
	SpriteAnimation::SpriteAnimation(uint32_t framesPerSecond, const std::initializer_list<KeyFrame>& keyFrames)
		:
		m_CurrentTime(0.0f),
		m_FrameLength(0.0f),
		m_CurrentKey(0),
		m_FPS(framesPerSecond),
		m_KeyFrames(keyFrames)
	{
	}
	void SpriteAnimation::Update(float dt)
	{
		m_CurrentTime += dt;
		if (m_CurrentTime >= m_KeyFrames.size() * m_FrameLength)
			m_CurrentTime = 0.0f;

		m_CurrentKey = (size_t)std::floor((m_CurrentTime / m_FrameLength) * m_KeyFrames.size());
	}
	void SpriteAnimation::SetFPS(uint32_t fps)
	{
		m_FPS = fps;
		m_FrameLength = 1.0f / m_FPS;
	}
	void SpriteAnimation::DeleteKeyFrame(size_t index)
	{
		XYZ_ASSERT(index < m_KeyFrames.size(), "Deleting frame out of range");
		m_KeyFrames.erase(m_KeyFrames.begin() + index);
	}
	const KeyFrame& SpriteAnimation::GetKeyFrame(size_t index) const
	{
		XYZ_ASSERT(index < m_KeyFrames.size(), "Frame index out of range");
		return m_KeyFrames[index];
	}
}