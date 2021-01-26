#include "stdafx.h"
#include "InGuiFactory.h"

namespace XYZ {

	static glm::vec2 GenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& pos, const glm::vec2& size, InGuiMesh& mesh, uint32_t textureID, uint32_t maxCount)
	{
		if (!source)
			return { 0.0f, 0.0f };

		float width = 0.0f;

		float xCursor = 0.0f;
		float yCursor = 0.0f;
		
		uint32_t counter = 0;
		while (source[counter] != '\0' && counter < maxCount)
		{
			auto& character = font->GetCharacter(source[counter]);
			if (source[counter] == '\n')
			{
				width = xCursor;
				yCursor += font->GetLineHeight();
				xCursor = 0.0f;
				counter++;
				continue;
			}

			if (xCursor + (float)character.XAdvance >= size.x)
				break;

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
			mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID });

			xCursor += character.XAdvance;
			counter++;
		}
		if (width < xCursor)
			width = xCursor;

		return { width, yCursor + font->GetLineHeight() };
	}

	void InGuiFactory::GenerateWindow(const char* text, InGuiWindow& window, const glm::vec4& color, const InGuiRenderData& renderData, Ref<SubTexture> subTexture, uint32_t textureID)
	{
		const glm::vec4& oldTex = subTexture->GetTexCoords();
		glm::vec4 texCoords = {
			oldTex.x, oldTex.w, oldTex.z, oldTex.y
		};
		window.Mesh.Quads.clear();
		window.Mesh.Lines.clear();
		glm::vec2 textOffset = { 7.0f, 7.0f };
		glm::vec2 textPosition = { window.Position.x + textOffset.x, window.Position.y + InGuiWindow::PanelHeight - textOffset.y };
		glm::vec2 textSize = { window.Size.x - textOffset.x - InGuiWindow::PanelHeight, window.Size.y - textOffset.y };
		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		if (!IS_SET(window.Flags, InGuiWindowFlags::Collapsed))
		{
			window.Mesh.Quads.push_back(
				{
					color,
					texCoords,
					{window.Position, InGuiWindow::PanelHeight },
					window.Size,
					textureID
				});
		}
		window.Mesh.Quads.push_back(
			{
				color,
				renderData.SubTexture[InGuiRenderData::BUTTON]->GetTexCoords(),
				{window.Position, 0.0f},
				{window.Size.x, InGuiWindow::PanelHeight },
				InGuiRenderData::TextureID
			});
		window.Mesh.Quads.push_back(
			{
				color,
				renderData.SubTexture[InGuiRenderData::MIN_BUTTON]->GetTexCoords(),
				{window.Position.x + window.Size.x - InGuiWindow::PanelHeight, window.Position.y, 0.0f},
				{InGuiWindow::PanelHeight,InGuiWindow::PanelHeight },
				InGuiRenderData::TextureID
			});	
		GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			textPosition, textSize, window.Mesh, InGuiRenderData::FontTextureID, 1000
		);
		
	}
	void InGuiFactory::GenerateQuad(InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex)
	{
		const glm::vec4& oldTex = renderData.SubTexture[subTextureIndex]->GetTexCoords();
		glm::vec4 texCoords = {
			oldTex.x, oldTex.w, oldTex.z, oldTex.y
		};
		window.Mesh.Quads.push_back(
			{
				color,
				texCoords,
				{position, 0.0f},
				size,
				InGuiRenderData::TextureID
			});
	}
	void InGuiFactory::GenerateQuad(InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, Ref<SubTexture> subTexture, uint32_t textureID)
	{
		const glm::vec4& oldTex = subTexture->GetTexCoords();
		glm::vec4 texCoords = {
			oldTex.x, oldTex.w, oldTex.z, oldTex.y
		};
		window.Mesh.Quads.push_back(
			{
				color,
				texCoords,
				{position, 0.0f},
				size,
				textureID
			});
	}
	glm::vec2 InGuiFactory::GenerateQuadWithText(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex)
	{
		GenerateQuad(window, color, size, position, renderData.SubTexture[subTextureIndex], InGuiRenderData::TextureID);
		glm::vec2 textPosition = position;
		glm::vec2 textSize = { 
			window.Size.x - window.Layout.RightPadding - size.x
			- (position.x - window.Position.x), 
			window.Size.y 
		};

		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		size_t oldMeshSize = window.Mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, textSize, window.Mesh, InGuiRenderData::FontTextureID, 1000
		);

		glm::vec2 textOffset = { 7.0f, 0.0f };
		
		for (size_t i = oldMeshSize; i < window.Mesh.Quads.size(); ++i)
		{
			window.Mesh.Quads[i].Position.x += std::floor(size.x + textOffset.x);
			window.Mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}


		float height = size.y;
		if (height < genSize.y)
			height = genSize.y;

		return glm::vec2(size.x + genSize.x + textOffset.x, height);
	}
	glm::vec2 InGuiFactory::GenerateQuadWithTextLeft(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex)
	{
		GenerateQuad(window, color, size, position, renderData.SubTexture[subTextureIndex], InGuiRenderData::TextureID);

		glm::vec2 textOffset = { 7.0f, 0.0f };
		glm::vec2 textPosition = position;
		glm::vec2 textSize = {size.x - textOffset.x, window.Size.y };
		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		size_t oldMeshSize = window.Mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, textSize, window.Mesh, InGuiRenderData::FontTextureID, 1000
		);


		for (size_t i = oldMeshSize; i < window.Mesh.Quads.size(); ++i)
		{
			window.Mesh.Quads[i].Position.x += std::floor(textOffset.x);
			window.Mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}


		float height = size.y;
		if (height < genSize.y)
			height = genSize.y;

		return glm::vec2(size.x, height);
	}
	glm::vec2 InGuiFactory::GenerateTextCentered(const char* text, InGuiWindow& window, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData, uint32_t maxCount)
	{
		size_t oldMeshSize = window.Mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, size, window.Mesh, InGuiRenderData::FontTextureID, maxCount
		);

		for (size_t i = oldMeshSize; i < window.Mesh.Quads.size(); ++i)
		{
			window.Mesh.Quads[i].Position.x += std::floor((size.x / 2.0f) - (genSize.x / 2.0f));
			window.Mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}
		return genSize;
	}
	glm::vec2 InGuiFactory::GenerateText(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData)
	{
		return GenerateTextMesh(
			text, renderData.Font, color,
			position, size, window.Mesh, InGuiRenderData::FontTextureID, 1000
		);
	}
}