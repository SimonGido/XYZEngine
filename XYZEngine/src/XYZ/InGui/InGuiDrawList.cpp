#include "stdafx.h"
#include "InGuiDrawList.h"

#include "InGuiUtil.h"

#include "XYZ/Renderer/CustomRenderer2D.h"

namespace XYZ {

	InGuiRect::InGuiRect()
		:
		Min(0.0f),
		Max(0.0f)
	{
	}
	InGuiRect::InGuiRect(const glm::vec2& min, const glm::vec2& max)
		:
		Min(min),
		Max(max)
	{
	}
	void InGuiRect::Translate(const glm::vec2& translation)
	{
		Min += translation;
		Max += translation;
	}
	bool InGuiRect::Overlaps(const InGuiRect& other) const
	{
		return other.Min.y <  Max.y&& other.Max.y >  Min.y
			&& other.Min.x <  Max.x&& other.Max.x >  Min.x;
	}
	bool InGuiRect::Overlaps(const glm::vec2& point) const
	{
		return (Max.x > point.x &&
			    Min.x < point.x &&
			    Max.y > point.y &&
			    Min.y < point.y);
	}

	void InGuiDrawList::PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& position, const glm::vec2& size, uint32_t textureID, uint32_t clipID)
	{
		m_Quads.push_back({ color, texCoord, glm::vec3(position,0.0f), size, textureID, clipID });
	}
	void InGuiDrawList::PushLine(const glm::vec4& color, const glm::vec2& p0, const glm::vec2& p1, uint32_t clipID)
	{
		m_Lines.push_back({ color, glm::vec3(p0,0.0f), glm::vec3(p1,0.0f), clipID });
	}
	void InGuiDrawList::PushText(const char* text, const glm::vec2& position, const glm::vec4& color, uint32_t textureID, const Ref<Font>& font, uint32_t clipID)
	{
		Util::GenerateTextMesh(text, font, color, position, m_Quads, textureID, clipID);
	}
	void InGuiDrawList::SubmitToRenderer()
	{
		for (auto& quad : m_Quads)
			CustomRenderer2D::SubmitQuadNotCentered(quad.Position, quad.Size, quad.TexCoord, quad.Color, (float)quad.TextureID, (float)quad.ClipID);
	
		for (auto& line : m_Lines)
			CustomRenderer2D::SubmitLine(line.P0, line.P1, line.Color, (float)line.ClipID);
	}
	void InGuiDrawList::Clear()
	{
		m_Quads.clear();
		m_Lines.clear();
	}
}