#pragma once
#include "XYZ/ECS/Component.h"
#include "SubTexture2D.h"

namespace XYZ {

	
	class SpriteAnimation : public Type<SpriteAnimation>
	{
	public:
		SpriteAnimation(unsigned int framesPerSecond,const std::initializer_list<Ref<SubTexture2D>>& keyFrames);

		void Update(float dt);
		void SetFPS(unsigned int fps);
		void DeleteKeyFrame(size_t index);

		const Ref<SubTexture2D>& GetKeyFrame(size_t index) const;
		const Ref<SubTexture2D>& GetCurrentKeyFrame() const { return m_KeyFrames[m_CurrentKey]; }
		const std::vector<Ref<SubTexture2D>>& GetKeyFrames() const { return m_KeyFrames; }
	private:
		float m_CurrentTime;
		float m_FrameLength;
		size_t m_CurrentKey;
		unsigned int m_FPS;
		std::vector<Ref<SubTexture2D>> m_KeyFrames;
	};

}