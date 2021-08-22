#pragma once
#include "Track.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {

	class SpriteRendererTrack : public Track
	{
	public:
		SpriteRendererTrack(SceneEntity entity);

		virtual bool  Update(float time) override;
		virtual void  Reset() override;
		virtual float Length() const override;

	
		void AddKeyFrame(const KeyFrame<Ref<SubTexture>>& spriteKey);
		void AddKeyFrame(const KeyFrame<glm::vec4>& colorKey);

	private:
		Property<Ref<SubTexture>> m_SpriteProperty;
		Property<glm::vec4>		  m_ColorProperty;
	};
}