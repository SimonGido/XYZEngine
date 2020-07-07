#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/RenderComponent.h"
#include "XYZ/Event/GuiEvent.h"
#include "Widget.h"

namespace XYZ {

	class Image : public Widget,
				  public RenderComponent, 
				  Type<Image>
	{
	public:
		Image(
			const glm::vec2& position,
			const glm::vec2& size,
			const glm::vec4& color,
			int32_t textureID,
			Ref<SubTexture2D> subTexture,
			Ref<Material> material,
			SortLayerID layer,
			bool visible
		);

		virtual ~Image() = default;

		void SetSubTexture(Ref<SubTexture2D> subTexture);
		void SetQuad(const Quad & quad);
		void SetColor(const glm::vec4& color);

		void SetNormalColor(const glm::vec4& color) { m_NormalColor = color; };
		void SetClickColor(const glm::vec4& color) { m_ClickColor = color; };
		void SetHooverColor(const glm::vec4& color) { m_HooverColor = color; };
		
		void ClickCallback(ClickEvent& e) { SetColor(m_ClickColor); };
		void HooverCallback(HooverEvent& e) { SetColor(m_HooverColor); }
		void ReleaseCallback(ReleaseEvent& e) { SetColor(m_NormalColor); }
		void UnHooverCallback(UnHooverEvent& e) { SetColor(m_NormalColor); }

		virtual const Quad* GetRenderData() const override { return &m_Quad; }
		virtual WidgetType GetWidgetType() override { return WidgetType::Image; }
	private:
		Quad m_Quad;
		Ref<SubTexture2D> m_SubTexture;

		glm::vec4 m_ClickColor;
		glm::vec4 m_HooverColor;
		glm::vec4 m_NormalColor;
	};
}