#pragma once
#include "Track.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {

	class SpriteRendererTrack : public Track
	{
	public:
		SpriteRendererTrack(SceneEntity entity);

		virtual bool     Update(uint32_t frame) override;
		virtual void     Reset() override;
		virtual uint32_t Length() const override;

	
		void AddKeyFrame(const KeyFrame<Ref<SubTexture>>& spriteKey);
		void AddKeyFrame(const KeyFrame<glm::vec4>& colorKey);
		void RemoveSpriteKeyFrame(uint32_t frame);
		void RemoveColorKeyFrame(uint32_t frame);
	private:
		virtual void updatePropertyCurrentKey(uint32_t frame) override;
	private:
		Property<Ref<SubTexture>> m_SpriteProperty;
		Property<glm::vec4>		  m_ColorProperty;
	};
}