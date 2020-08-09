#pragma once
#include "XYZ/ECS/Component.h"
#include "SubTexture2D.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum KeyFrameProperty
	{
		SubTexture	= 1 << 0,
		Translation = 1 << 1,
		Rotation	= 1 << 2,
		Scale		= 1 << 3
	};

	struct KeyFrame
	{
		Ref<SubTexture2D> SubTexture;
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		uint8_t Properties = 0;
	};
	
	class SpriteAnimation : public Type<SpriteAnimation>
	{
	public:
		SpriteAnimation(uint32_t framesPerSecond,const std::initializer_list<KeyFrame>& keyFrames);

		void Update(float dt);
		void SetFPS(uint32_t fps);
		void DeleteKeyFrame(size_t index);

		const KeyFrame& GetKeyFrame(size_t index) const;
		const KeyFrame& GetCurrentKeyFrame() const { return m_KeyFrames[m_CurrentKey]; }
		const std::vector<KeyFrame>& GetKeyFrames() const { return m_KeyFrames; }
	private:
		float m_CurrentTime;
		float m_FrameLength;
		size_t m_CurrentKey;
		uint32_t m_FPS;
		std::vector<KeyFrame> m_KeyFrames;
	};

}