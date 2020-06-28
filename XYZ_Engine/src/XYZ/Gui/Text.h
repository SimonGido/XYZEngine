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
			const glm::vec2& size,
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

	private:
		void rebuild();

	private:
		Ref<Font> m_Font;
		std::string m_Text;

		glm::vec2 m_Position;
		glm::vec2 m_Size;
		glm::vec4 m_Color;

		uint32_t m_Cursor = 0;
		std::vector<Quad> m_Quads;
	};
}