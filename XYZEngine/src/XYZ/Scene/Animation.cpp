#include "stdafx.h"
#include "Animation.h"

#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>

namespace XYZ {

	static float UpdateFloat(const Property<float>::KeyFrame& begin, const Property<float>::KeyFrame& end, float currentTime)
	{
		float scale = (currentTime - begin.Time) / (end.Time - begin.Time);
		float valueDiff = end.Value - begin.Value;
	
		return begin.Value + (valueDiff * scale);
	}

	static glm::vec3 UpdateVec3(const Property<glm::vec3>::KeyFrame& begin, const Property<glm::vec3>::KeyFrame& end, float currentTime)
	{
		float scale = (currentTime - begin.Time) / (end.Time - begin.Time);
		glm::vec3 valueDiff = end.Value - begin.Value;
		
		glm::vec3 result = { 0,0,0 };
		result.x = begin.Value.x + (valueDiff.x * scale);
		result.y = begin.Value.y + (valueDiff.y * scale);
		result.z = begin.Value.z + (valueDiff.z * scale);

		return result;
	}

	static glm::vec4 UpdateVec4(const Property<glm::vec4>::KeyFrame& begin, const Property<glm::vec4>::KeyFrame& end, float currentTime)
	{
		float scale = (currentTime - begin.Time) / (end.Time - begin.Time);
		glm::vec4 valueDiff = end.Value - begin.Value;

		glm::vec4 result = { 0, 0, 0 ,0 };
		result.x = begin.Value.x + (valueDiff.x * scale);
		result.y = begin.Value.y + (valueDiff.y * scale);
		result.z = begin.Value.z + (valueDiff.z * scale);
		result.w = begin.Value.w + (valueDiff.w * scale);

		return result;
	}

	void Property<glm::vec3>::Update(float currentTime)
	{
		if (CurrentFrame < KeyFrames.size() - 1)
		{					
			ModifiedValue = UpdateVec3(KeyFrames[CurrentFrame], KeyFrames[CurrentFrame + 1], currentTime);
			if (currentTime >= KeyFrames[CurrentFrame + 1].Time)
				CurrentFrame++;
		}
	}

	void Property<glm::vec4>::Update(float currentTime)
	{
		if (CurrentFrame < KeyFrames.size() - 1)
		{
			ModifiedValue = UpdateVec4(KeyFrames[CurrentFrame], KeyFrames[CurrentFrame + 1], currentTime);
			if (currentTime >= KeyFrames[CurrentFrame + 1].Time)
				CurrentFrame++;
		}
	}

	void Property <Ref<SubTexture>>::Update(float currentTime)
	{
		if (CurrentFrame < KeyFrames.size() - 1)
		{
			ModifiedValue = KeyFrames[CurrentFrame].Value;
			if (currentTime >= KeyFrames[CurrentFrame + 1].Time)
				CurrentFrame++;
		}
	}

	Animation::Animation(float animLength, bool repeat)
		:
		AnimationLength(animLength),
		Repeat(repeat)
	{
	}
	Animation::~Animation()
	{
		for (auto prop : Properties)
			delete prop;
	}
	void Animation::Update(float dt)
	{
		if (CurrentTime < AnimationLength)
		{
			CurrentTime += dt;
			for (auto prop : Properties)
				prop->Update(CurrentTime);
		}
		else if (Repeat)
		{
			CurrentTime = 0.0f;
			for (auto prop : Properties)
				prop->CurrentFrame = 0;
		}
	}
}