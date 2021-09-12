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
		if (m_CurrentFrame >= m_NumFrames)
		{
			if (!m_Repeat)
				return;

			m_CurrentTime = 0.0f;
			m_CurrentFrame = 0;
			for (auto& track : m_Tracks)
				track->Reset();
		}
		for (auto& track : m_Tracks)
			track->Update(m_CurrentFrame);

		m_CurrentTime += ts;
		m_CurrentFrame = m_CurrentTime / m_FrameLength;
	}

	void Animation::UpdateLength()
	{
		for (const auto& track : m_Tracks)
			m_NumFrames = std::max(m_NumFrames, track->Length());
		SetCurrentFrame(std::min(m_CurrentFrame, m_NumFrames));
	}

	void Animation::SetCurrentFrame(uint32_t frame)
	{
		m_CurrentFrame = std::min(m_NumFrames, frame);
		m_CurrentTime = m_CurrentFrame * m_FrameLength;
		for (auto& track : m_Tracks)
		{
			track->updatePropertyCurrentKey(m_CurrentFrame);
			track->Update(m_CurrentFrame);
		}
	}

}