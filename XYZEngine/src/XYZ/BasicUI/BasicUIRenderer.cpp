#include "stdafx.h"
#include "BasicUIRenderer.h"

#include "BasicUITypes.h"
#include "BasicUI.h"
#include "BasicUIHelper.h"

namespace XYZ {
	namespace Helper {
		static void GenerateQuad(bUIMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const Ref<SubTexture>& subTexture, uint32_t textureID, uint32_t scissorID)
		{
			const glm::vec4& oldTex = subTexture->GetTexCoords();
			glm::vec4 texCoords = {
				oldTex.x, oldTex.w, oldTex.z, oldTex.y
			};
			mesh.Quads.push_back(
				{
					color,
					texCoords,
					{position, 0.0f},
					size,
					textureID,
					scissorID
				});
		}
		static void GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			bUIMesh& mesh,
			uint32_t textureID,
			uint32_t scissorID,
			uint32_t maxCharacters = UINT32_MAX
		)
		{
			if (!source)
				return;

			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0' && counter < maxCharacters)
			{
				auto& character = font->GetCharacter(source[counter]);
				if (source[counter] == '\n')
				{
					yCursor += font->GetLineHeight();
					xCursor = 0.0f;
					counter++;
					continue;
				}
				glm::vec2 charSize = {
					character.X1Coord - character.X0Coord,
					character.Y1Coord - character.Y0Coord
				};

				glm::vec2 charOffset = { character.XOffset, character.YOffset };
				glm::vec2 charPosition = { pos.x + xCursor + charOffset.x, pos.y + yCursor - charOffset.y };
				glm::vec4 charTexCoord = {
					(float)(character.X0Coord) / (float)(font->GetWidth()), (float)(character.Y0Coord) / (float)(font->GetHeight()),
					(float)(character.X1Coord) / (float)(font->GetWidth()), (float)(character.Y1Coord) / (float)(font->GetHeight())
				};

				mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID, scissorID });
				xCursor += character.XAdvance;
				counter++;
			}
		}
	}

	template <>
	void bUIRenderer::Submit<bUIButton>(const bUIButton& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUIImage>(const bUIImage& element, uint32_t& scissorID)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		if (element.ImageSubTexture.Raw())
		{
			uint32_t textureID = 0;
			for (size_t i = 0; i < m_CustomTextures.size(); ++i)
			{
				if (m_CustomTextures[i].Raw() == element.ImageSubTexture->GetTexture().Raw())
				{
					textureID = i + 2;
					break;
				}
			}
			if (!textureID)
			{
				textureID = m_CustomTextures.size() + 2;
				m_CustomTextures.push_back(element.ImageSubTexture->GetTexture());
			}
			Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, element.ImageSubTexture, textureID, scissorID);
		}
		else
			Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, bUI::GetConfig().GetSubTexture(bUIConfig::White), 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUIFloat>(const bUIFloat& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			uint32_t maxCharacters = UINT32_MAX;
			if (element.CutTextOutside)
				maxCharacters = bUIHelper::FindNumCharacterToFit(element.Size, element.GetBuffer(), font);
			glm::vec2 size = bUIHelper::FindTextSize(element.GetBuffer(), font, maxCharacters);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x = absolutePosition.x + ((element.Size.x - size.x) / 2.0f);
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.GetBuffer(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID, maxCharacters);
		}
	}

	template <>
	void bUIRenderer::Submit<bUIInt>(const bUIInt& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			uint32_t maxCharacters = UINT32_MAX;
			if (element.CutTextOutside)
				maxCharacters = bUIHelper::FindNumCharacterToFit(element.Size, element.GetBuffer(), font);
			glm::vec2 size = bUIHelper::FindTextSize(element.GetBuffer(), font, maxCharacters);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x = absolutePosition.x + ((element.Size.x - size.x) / 2.0f);
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.GetBuffer(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID, maxCharacters);
		}
	}

	template <>
	void bUIRenderer::Submit<bUIString>(const bUIString& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.GetValue().c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x = absolutePosition.x + ((element.Size.x - size.x) / 2.0f);
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.GetValue().c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
	}


	template <>
	void bUIRenderer::Submit<bUICheckbox>(const bUICheckbox& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUISlider>(const bUISlider& element, uint32_t& scissorID, const Ref<SubTexture>& sliderSubTexture, const Ref<SubTexture>& handleSubTexture, const glm::vec2& handlePosition, const float& value)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, sliderSubTexture, 0, scissorID);
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, { element.Size.y, element.Size.y } , handlePosition, handleSubTexture, 0, scissorID);
		{
			char buffer[10];
			sprintf(buffer, "%f", value);
			glm::vec2 size = bUIHelper::FindTextSize(buffer, font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x = absolutePosition.x + ((element.Size.x - size.x) / 2.0f);
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(buffer, font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
	}

	template <>
	void bUIRenderer::Submit<bUIWindow>(const bUIWindow& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture, const Ref<SubTexture>& minimizeSubTexture)
	{
		glm::vec2 panelSize = { element.Size.x, element.ButtonSize.y };
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		glm::vec2 absolutePanelPosition = absolutePosition - glm::vec2(0.0f, panelSize.y);
		Ref<Font> font = bUI::GetConfig().m_Font;
		if (element.ChildrenVisible)
			Helper::GenerateQuad(m_Mesh, element.Color, element.Size, absolutePosition, subTexture, 0, scissorID);
		
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, panelSize, absolutePanelPosition, subTexture, 0, scissorID);
		Helper::GenerateQuad(m_Mesh, element.Color, element.ButtonSize, absolutePanelPosition, minimizeSubTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePanelPosition;
		textPosition.x += element.ButtonSize.x + 2.0f;
		textPosition.y += std::floor(((element.ButtonSize.y - size.y) / 2.0f) + font->GetLineHeight());
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUIScrollbox>(const bUIScrollbox& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsoluteScrollPosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y += ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);

		absolutePosition.y = bUI::GetContext().ViewportSize.y - absolutePosition.y - element.Size.y;
		m_Mesh.Scissors.push_back({ absolutePosition.x, absolutePosition.y, element.Size.x, element.Size.y });
	}

	template <>
	void bUIRenderer::Submit<bUITree>(const bUITree& element, uint32_t& scissorID, const Ref<SubTexture>& closedSubTexture, const Ref<SubTexture>& openSubTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		
		element.Hierarchy.Traverse([&](void* parent, void* child)->bool {

			hUIHierarchyItem* childItem = static_cast<hUIHierarchyItem*>(child);
			glm::vec2 childAbsolutePosition = absolutePosition + childItem->GetCoords();
			if (parent)
			{
				hUIHierarchyItem* parentItem = static_cast<hUIHierarchyItem*>(parent);
				glm::vec2 parentAbsolutePosition = absolutePosition + parentItem->GetCoords();
				if (!parentItem->Open)
					return false;
				else
				{
					glm::vec3 lineStart = glm::vec3(parentAbsolutePosition.x + element.Size.x / 2.0f, parentAbsolutePosition.y + element.Size.y, 0.0f);
					glm::vec3 lineEnd = glm::vec3(childAbsolutePosition.x, childAbsolutePosition.y + element.Size.y / 2.0f, 0.0f);
					glm::vec3 lineMiddle = glm::vec3(lineStart.x, lineEnd.y, 0.0f);
					m_Mesh.Lines.push_back({ glm::vec4(1.0f), lineStart, lineMiddle });
					m_Mesh.Lines.push_back({ glm::vec4(1.0f), lineMiddle, lineEnd });
				}
			}

		
			if (childItem->Open)
				Helper::GenerateQuad(m_Mesh, childItem->Color, element.Size, childAbsolutePosition, openSubTexture, 0, scissorID);
			else
				Helper::GenerateQuad(m_Mesh, childItem->Color, element.Size, childAbsolutePosition, closedSubTexture, 0, scissorID);
			
			glm::vec2 size = bUIHelper::FindTextSize(childItem->Label.c_str(), font);
			glm::vec2 textPosition = childAbsolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y += ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(childItem->Label.c_str(), font, childItem->Color, textPosition, m_Mesh, 1, scissorID);
			return false;
		});
	}

	template <>
	void bUIRenderer::Submit<bUIDropdown>(const bUIDropdown& element, uint32_t& scissorID, const Ref<SubTexture>& quadSubTexture, const Ref<SubTexture>& arrowSubTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		{
			Helper::GenerateQuad(m_Mesh, element.Color, element.Size, absolutePosition, quadSubTexture, 0, scissorID);			
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += ((element.Size.x - size.x) / 2.0f);
			textPosition.y += ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, element.Color, textPosition, m_Mesh, 1, scissorID);
		}
		if (!element.ChildrenVisible)
			return;

		absolutePosition.y += element.Size.y;
		element.Hierarchy.Traverse([&](void* parent, void* child)->bool {

			hUIHierarchyItem* childItem = static_cast<hUIHierarchyItem*>(child);
			glm::vec2 childAbsolutePosition = absolutePosition + childItem->GetCoords();
			if (parent)
			{
				hUIHierarchyItem* parentItem = static_cast<hUIHierarchyItem*>(parent);
				glm::vec2 parentAbsolutePosition = absolutePosition + parentItem->GetCoords();
				if (!parentItem->Open)
					return false;
			}

			Helper::GenerateQuad(m_Mesh, childItem->Color, element.Size, childAbsolutePosition, quadSubTexture, 0, scissorID);
			if (element.Hierarchy.HasChildren(childItem->GetID()))
			{
				glm::vec2 arrowSize(element.Size.y, element.Size.y);
				glm::vec2 arrowPosition(childAbsolutePosition.x + element.Size.x - arrowSize.x, childAbsolutePosition.y);
				Helper::GenerateQuad(m_Mesh, childItem->Color, arrowSize, arrowPosition, arrowSubTexture, 0, scissorID);
			}
			glm::vec2 size = bUIHelper::FindTextSize(childItem->Label.c_str(), font);
			glm::vec2 textPosition = childAbsolutePosition;
			textPosition.x += 2.0f;
			textPosition.y += ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(childItem->Label.c_str(), font, childItem->Color, textPosition, m_Mesh, 1, scissorID);
			return false;
			});
	}


	template <>
	void bUIRenderer::Submit<bUIText>(const bUIText& element, uint32_t& scissorID)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Text.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x = absolutePosition.x + ((element.Size.x - size.x) / 2.0f);
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Text.c_str(), font, element.Color, textPosition, m_Mesh, 1, scissorID);
		}
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y = absolutePosition.y + ((element.Size.y - size.y) / 2.0f) + font->GetLineHeight();
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
	}

	void bUIRenderer::Begin()
	{
		m_Mesh.Quads.clear();
		m_Mesh.Lines.clear();
		m_Mesh.Scissors.clear();
		m_Mesh.Scissors.push_back({ 0.0f, 0.0f, bUI::GetContext().ViewportSize.x, bUI::GetContext().ViewportSize.y });
		m_CustomTextures.clear();
	}
	void bUIRenderer::BindCustomTextures()
	{
		uint32_t slot = 2;
		for (auto& texture : m_CustomTextures)
			texture->Bind(slot++);
	}
	void bUIRenderer::UpdateScissorBuffer(Ref<ShaderStorageBuffer> scissorBuffer)
	{
		scissorBuffer->Update(m_Mesh.Scissors.data(), m_Mesh.Scissors.size() * sizeof(bUIScissor));
	}
}