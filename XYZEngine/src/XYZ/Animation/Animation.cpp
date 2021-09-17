#include "stdafx.h"
#include "Animation.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

	Animation::Animation()
		:
		m_NumFrames(0),
		m_CurrentFrame(0),
		m_CurrentTime(0.0f),
		m_FrameLength(0.05f),
		m_Repeat(true)
	{
	}

	void Animation::Update(Timestep ts)
	{
		for (auto& animatable : m_Animatables)
			animatable.SetReference();
		
		if (m_CurrentFrame >= m_NumFrames)
		{
			if (!m_Repeat)
				return;

			m_CurrentTime = 0.0f;
			m_CurrentFrame = 0;
		}

		m_CurrentTime += ts;
		m_CurrentFrame = static_cast<uint32_t>(std::floor(m_CurrentTime / m_FrameLength));
	}

	void Animation::UpdateLength()
	{
		
	}

	void Animation::SetCurrentFrame(uint32_t frame)
	{
		m_CurrentFrame = std::min(m_NumFrames, frame);
		m_CurrentTime = m_CurrentFrame * m_FrameLength;
		
	}

}