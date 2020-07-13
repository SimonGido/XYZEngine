#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "RenderComponent.h"


namespace XYZ {

	class SpriteRenderComponent : public RenderComponent, 
								  public Type<SpriteRenderComponent>
	{
	public:
		SpriteRenderComponent(
			int32_t textureID,
			const glm::vec4& color,
			const Ref<SubTexture2D>& subTexture,
			const Ref<Material>& material,
			SortLayerID layer,
			bool visible = true
		);
		virtual ~SpriteRenderComponent() = default;

		void SetSubTexture(const Ref<SubTexture2D>& subTexture);
		void SetQuad(const Quad& quad);

		virtual const Quad* GetRenderData() const { return &m_Quad; };
	private:
		Quad m_Quad;
		Ref<SubTexture2D> m_SubTexture;
	};

}