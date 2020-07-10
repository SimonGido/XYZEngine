#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "RenderComponent.h"


namespace XYZ {

	class SpriteRenderComponent : public RenderComponent, Type<SpriteRenderComponent>
	{
	public:
		SpriteRenderComponent(
			const glm::vec2& position,
			const glm::vec2& size,
			const glm::vec4& color,
			int32_t textureID,
			Ref<SubTexture2D> subTexture,
			Ref<Material> material,
			SortLayerID layer,
			bool visible
		);
		virtual ~SpriteRenderComponent() = default;

		void SetSubTexture(Ref<SubTexture2D> subTexture);
		void SetQuad(const Quad& quad);

		virtual const Quad* GetRenderData() const { return &m_Quad; };
	private:
		Quad m_Quad;
		Ref<SubTexture2D> m_SubTexture;
	};

}