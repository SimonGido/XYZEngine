#pragma once
#include "XYZ/Renderer/Font.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct InGuiRect
	{
		InGuiRect();
		InGuiRect(const glm::vec2& min, const glm::vec2& max);

		void Translate(const glm::vec2& translation);
		bool Overlaps(const InGuiRect& other) const;
		bool Overlaps(const glm::vec2& point) const;

		glm::vec2 Min;
		glm::vec2 Max;
	};

	struct InGuiQuad
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec2 Size;
		uint32_t  TextureID;
		uint32_t  ClipID;
	};

	struct InGuiLine
	{
		glm::vec4 Color;
		glm::vec3 P0;
		glm::vec3 P1;
		uint32_t  ClipID;
	};

	class InGuiDrawList
	{
	public:
		void PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& position, const glm::vec2& size, uint32_t textureID, uint32_t clipID);
		void PushLine(const glm::vec4& color, const glm::vec2& p0, const glm::vec2& p1, uint32_t clipID);
		void PushText(const char* text, const glm::vec2& position, const glm::vec4& color, uint32_t textureID, const Ref<Font>& font, uint32_t clipID);

		void SubmitToRenderer();
		void Clear();

		std::vector<InGuiQuad> m_Quads;
		std::vector<InGuiLine> m_Lines;
	};
}