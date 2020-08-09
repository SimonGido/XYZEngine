#include "stdafx.h"
#include "Animation.h"

#include <glm/glm.hpp>

namespace XYZ {


	static glm::vec3 TransitionVec3(const KeyFrame<glm::vec3>& begin, const KeyFrame<glm::vec3>& end, float currentTime)
	{
		float endBeginDiff = end.Time - begin.Time;
		float beginTimeDiff = currentTime - begin.Time;
		float scale = endBeginDiff / beginTimeDiff;
		
		glm::vec3 valueDiff = end.Value - begin.Value;

		glm::vec3 result;
		result.x = (begin.Value.x + valueDiff.x) * scale;
		result.y = (begin.Value.y + valueDiff.y) * scale;
		result.z = (begin.Value.z + valueDiff.z) * scale;

		return result;
	}

	void Property<glm::vec3>::Transition(float time, uint32_t fps)
	{
		if (CurrentFrame < KeyFrames.size() - 1 && time >= KeyFrames[CurrentFrame].Time)
		{
			if (ModifiedValue)
				*ModifiedValue = TransitionVec3(KeyFrames[CurrentFrame], KeyFrames[CurrentFrame + 1], time);
			
			
			CurrentFrame++;
		}
	}
	Animation::Animation(float animLength, uint32_t fps, bool repeat)
		:
		m_AnimationLength(animLength),
		m_FPS(fps),
		m_Repeat(repeat)
	{
	}
	Animation::~Animation()
	{
		for (auto prop : m_Properties)
			delete prop;
	}
	void Animation::Update(float dt)
	{
		if (m_CurrentTime < m_AnimationLength)
		{
			m_CurrentTime += dt;
			for (auto prop : m_Properties)
				prop->Transition(dt, m_FPS);
		}
		else if (m_Repeat)
		{
			m_CurrentTime = 0.0f;
			for (auto prop : m_Properties)
				prop->CurrentFrame = 0;
		}
	}
}