#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/RenderComponent.h"
#include "Font.h"

namespace XYZ {

	class Text : public RenderComponent , Type<Text>
	{
	public:
		Text( 	
			const std::string& text,
			const glm::vec2& position,
			const glm::vec4& color,
			int32_t textureID,
			Ref<Font> font,
			Ref<Material> material,
			SortLayerID layer,
			bool visible
		);

		virtual ~Text() = default;
		
		void Pop();
		void Push(char c);
		void Push(const std::string& text);
		void Clear();

		virtual size_t GetCountQuads() const { return m_Quads.size(); }
		virtual const Quad* GetRenderData() const override { return m_Quads.data(); }
	private:
		void rebuild();

	private:
		Ref<Font> m_Font;
		std::string m_Text;

		glm::vec2 m_Position;
		glm::vec4 m_Color;
		int32_t m_TextureID;

		uint32_t m_CursorX = 0;
		uint32_t m_CursorY = 0;
		std::vector<Quad> m_Quads;
	};
}