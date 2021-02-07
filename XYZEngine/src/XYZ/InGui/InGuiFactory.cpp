#include "stdafx.h"
#include "InGuiFactory.h"

namespace XYZ {

	static glm::vec2 GenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& pos, const glm::vec2& size, InGuiMesh& mesh, uint32_t textureID, uint32_t maxCount, uint32_t scissorIndex)
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

			mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID, scissorIndex });

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
		window.OverlayMesh.Quads.clear();
		window.OverlayMesh.Lines.clear();
		window.ScrollableMesh.Quads.clear();
		window.ScrollableMesh.Lines.clear();

		glm::vec2 textOffset = { 7.0f, 7.0f };
		glm::vec2 textPosition = { window.Position.x + textOffset.x, window.Position.y + InGuiWindow::PanelHeight - textOffset.y };
		glm::vec2 textSize = { window.Size.x - textOffset.x - InGuiWindow::PanelHeight, window.Size.y - textOffset.y };
		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		if (!IS_SET(window.Flags, InGuiWindowFlags::Collapsed))
		{
			GenerateFrame(window.Mesh, window.Position, window.Size, renderData);
			window.Mesh.Quads.push_back(
				{
					glm::vec4(1.0f),
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
			textPosition, textSize, window.Mesh, InGuiRenderData::FontTextureID, 1000, 0
		);
		
	}
	void InGuiFactory::GenerateQuad(InGuiMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex, uint32_t scissorIndex)
	{
		const glm::vec4& oldTex = renderData.SubTexture[subTextureIndex]->GetTexCoords();
		glm::vec4 texCoords = {
			oldTex.x, oldTex.w, oldTex.z, oldTex.y
		};
		mesh.Quads.push_back(
			{
				color,
				texCoords,
				{position, 0.0f},
				size,
				InGuiRenderData::TextureID,
				scissorIndex
			});
	}
	void InGuiFactory::GenerateQuad(InGuiMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, Ref<SubTexture> subTexture, uint32_t textureID, uint32_t scissorIndex)
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
				scissorIndex
			});
	}
	glm::vec2 InGuiFactory::GenerateQuadWithText(const char* text, const InGuiWindow& window, InGuiMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex, uint32_t scissorIndex)
	{
		GenerateQuad(mesh, color, size, position, renderData.SubTexture[subTextureIndex], InGuiRenderData::TextureID, scissorIndex);
		glm::vec2 textPosition = position;
		glm::vec2 textSize = { 
			window.Size.x - window.Layout.RightPadding - size.x
			- (position.x - window.Position.x), 
			window.Size.y 
		};

		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		size_t oldMeshSize = mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, textSize, mesh, InGuiRenderData::FontTextureID, 1000, scissorIndex
		);

		glm::vec2 textOffset = { 7.0f, 0.0f };
		
		for (size_t i = oldMeshSize; i < mesh.Quads.size(); ++i)
		{
			mesh.Quads[i].Position.x += std::floor(size.x + textOffset.x);
			mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}


		float height = size.y;
		if (height < genSize.y)
			height = genSize.y;

		return glm::vec2(size.x + genSize.x + textOffset.x, height);
	}
	glm::vec2 InGuiFactory::GenerateQuadWithTextLeft(const char* text, const InGuiWindow& window, InGuiMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex, uint32_t scissorIndex)
	{
		GenerateQuad(mesh, color, size, position, renderData.SubTexture[subTextureIndex], InGuiRenderData::TextureID, scissorIndex);

		glm::vec2 textOffset = { 7.0f, 0.0f };
		glm::vec2 textPosition = position;
		glm::vec2 textSize = {size.x - textOffset.x, window.Size.y };
		textPosition.x = std::floor(textPosition.x);
		textPosition.y = std::floor(textPosition.y);

		size_t oldMeshSize = mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, textSize, mesh, InGuiRenderData::FontTextureID, 1000, scissorIndex
		);


		for (size_t i = oldMeshSize; i < mesh.Quads.size(); ++i)
		{
			mesh.Quads[i].Position.x += std::floor(textOffset.x);
			mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}


		float height = size.y;
		if (height < genSize.y)
			height = genSize.y;

		return glm::vec2(size.x, height);
	}
	glm::vec2 InGuiFactory::GenerateTextCentered(const char* text, const InGuiWindow& window, InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData, uint32_t maxCount, uint32_t scissorIndex)
	{
		size_t oldMeshSize = mesh.Quads.size();
		glm::vec2 genSize = GenerateTextMesh(
			text, renderData.Font, renderData.Color[InGuiRenderData::DEFAULT_COLOR],
			position, size, mesh, InGuiRenderData::FontTextureID, maxCount, scissorIndex
		);

		for (size_t i = oldMeshSize; i < mesh.Quads.size(); ++i)
		{
			mesh.Quads[i].Position.x += std::floor((size.x / 2.0f) - (genSize.x / 2.0f));
			mesh.Quads[i].Position.y += std::floor((size.y / 2.0f) + (genSize.y / 2.0f));
		}
		return genSize;
	}
	glm::vec2 InGuiFactory::GenerateText(const char* text, InGuiMesh& mesh, const glm::vec4& color, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData, uint32_t scissorIndex)
	{
		return GenerateTextMesh(
			text, renderData.Font, color,
			position, size, mesh, InGuiRenderData::FontTextureID, 1000, scissorIndex
		);
	}
	void InGuiFactory::GenerateFrame(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData)
	{
		InGuiLine line;
		line.Color = renderData.Color[InGuiRenderData::LINE_COLOR];

		line.P0 = glm::vec3(position, 0.0f);
		line.P1 = glm::vec3(position.x + size.x, position.y, 0.0f);
		mesh.Lines.push_back(line);

		line.P0 = glm::vec3(position.x + size.x, position.y, 0.0f);
		line.P1 = glm::vec3(position.x + size.x, position.y + size.y, 0.0f);
		mesh.Lines.push_back(line);

		line.P0 = glm::vec3(position.x + size.x, position.y + size.y, 0.0f);
		line.P1 = glm::vec3(position.x, position.y + size.y, 0.0f);
		mesh.Lines.push_back(line);

		line.P0 = glm::vec3(position.x, position.y + size.y, 0.0f);
		line.P1 = glm::vec3(position, 0.0f);
		mesh.Lines.push_back(line);
	}
	void InGuiFactory::GenerateDockNode(InGuiDockNode& node, InGuiMesh& mesh, const glm::vec2& quadSize, const InGuiRenderData& renderData)
	{
		glm::vec4 quadColor = glm::vec4(1.5f, 1.8f, 2.9f, 1.0f);
		glm::vec2 middlePos = node.Data.Position + ((node.Data.Size - quadSize) / 2.0f);
		glm::vec2 leftPos = node.Data.Position + glm::vec2(0.0f, (node.Data.Size.y - quadSize.y) / 2.0f);
		glm::vec2 rightPos = node.Data.Position + glm::vec2(node.Data.Size.x - quadSize.x, (node.Data.Size.y - quadSize.y) / 2.0f);
		glm::vec2 topPos = node.Data.Position + glm::vec2((node.Data.Size.x - quadSize.x) / 2.0f, 0.0f);
		glm::vec2 bottomPos = node.Data.Position + glm::vec2((node.Data.Size.x - quadSize.x) / 2.0f, node.Data.Size.y - quadSize.y);
		GenerateQuad(
			mesh, quadColor, quadSize, middlePos, 
			renderData.SubTexture[InGuiRenderData::BUTTON], InGuiRenderData::TextureID, 0);
		GenerateQuad(
			mesh, quadColor, quadSize, leftPos, 
			renderData.SubTexture[InGuiRenderData::BUTTON], InGuiRenderData::TextureID, 0);
		GenerateQuad(
			mesh, quadColor, quadSize, rightPos, 
			renderData.SubTexture[InGuiRenderData::BUTTON], InGuiRenderData::TextureID, 0);
		GenerateQuad(
			mesh, quadColor, quadSize, topPos,
			renderData.SubTexture[InGuiRenderData::BUTTON], InGuiRenderData::TextureID, 0);
		GenerateQuad(
			mesh, quadColor, quadSize, bottomPos,
			renderData.SubTexture[InGuiRenderData::BUTTON], InGuiRenderData::TextureID, 0);
	}
}