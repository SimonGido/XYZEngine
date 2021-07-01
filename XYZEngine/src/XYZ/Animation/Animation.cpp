#include "stdafx.h"
#include "Animation.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

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
			for (auto& track : m_Tracks)
				track->Reset();
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

}