#include "stdafx.h"
#include "InGuiFactory.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static constexpr uint8_t sc_MaxFloatValueBufferSize = 32;

	static void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const uint32_t textureID, const glm::vec4& color, float tilingFactor = 1.0f)
	{
		constexpr size_t quadVertexCount = 4;
		glm::vec2 texCoords[quadVertexCount] = {
		{texCoord.x,texCoord.y},
		{texCoord.z,texCoord.y},
		{texCoord.z,texCoord.w},
		{texCoord.x,texCoord.w}
		};

		glm::vec4 quadVertexPositions[4] = {
			{ position.x ,		  position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y + size.y, 0.0f, 1.0f },
			{ position.x ,        position.y + size.y, 0.0f, 1.0f }
		};

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			mesh.Vertices.push_back({ color, quadVertexPositions[i], texCoords[i], (float)textureID, tilingFactor });
		}
	}

	static void GenerateInGuiQuad(InGuiMesh& mesh, InGuiVertex* vertices, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			mesh.Vertices.push_back(vertices[i]);
	}

	static void GenerateInGuiImage(InGuiMesh& mesh,std::vector<TextureRendererIDPair>& texturePairs, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, float tilingFactor)
	{
		uint32_t textureID = texturePairs.size() + InGuiRenderConfiguration::DefaultTextureCount;
		for (auto& pair : texturePairs)
		{
			if (pair.RendererID == rendererID)
			{
				textureID = pair.TextureID;
				break;
			}
		}
		texturePairs.push_back({ textureID,rendererID });
		GenerateInGuiQuad(mesh, position, size, texCoord, textureID, color, tilingFactor);
	}


	static TextInfo GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const char* str, const glm::vec2& position, float length, uint32_t textureID, const glm::vec4& color)
	{
		TextInfo textInfo;
		float xCursor = 0.0f;
		float yCursor = 0.0f;
		uint32_t i = 0;
		while (str[i] != '\0')
		{
			auto& character = font->GetCharacter(str[i]);
			glm::vec2 size = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};

			if (xCursor + size.x >= length)
				break;
			glm::vec2 offset = { character.XOffset, size.y - character.YOffset };
			glm::vec2 pos = { position.x + xCursor + offset.x, position.y + yCursor - offset.y };
			glm::vec4 coords = {
				(float)character.X0Coord / (float)font->GetWidth(), (float)character.Y0Coord / (float)font->GetHeight(),
				(float)character.X1Coord / (float)font->GetWidth(), (float)character.Y1Coord / (float)font->GetHeight()
			};
			mesh.Vertices.push_back({ color, { pos.x , pos.y, 0.0f },                  {coords.x, coords.w}, (float)textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y, 0.0f, },         {coords.z, coords.w}, (float)textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y + size.y, 0.0f }, {coords.z, coords.y}, (float)textureID });
			mesh.Vertices.push_back({ color, { pos.x , pos.y + size.y, 0.0f},          {coords.x, coords.y}, (float)textureID });
			
			xCursor += character.XAdvance;
			textInfo.Size.x = xCursor;
			textInfo.Count++;
			if (textInfo.Size.y < size.y)
				textInfo.Size.y = size.y;

			i++;
		}
		return textInfo;
	}
	
	static void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, uint32_t offset, uint32_t count)
	{
		for (uint32_t i = offset; i < count + offset; ++i)
		{
			vertices[i].Position.x += position.x;
			vertices[i].Position.y += position.y;
		}
	}

	void InGuiFactory::GenerateWindow(const char* name,InGuiWindow& window, const InGuiRenderConfiguration& renderConfig)
	{
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.OverlayLineMesh.Vertices.clear();
		window.OverlayMesh.Vertices.clear();

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & InGuiWindowFlag::Moved)
			panelColor = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 closeButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };
		glm::vec2 minButtonPos = closeButtonPos - glm::vec2(InGuiWindow::PanelSize, 0.0f);
		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER]->GetTexCoords(), renderConfig.TextureID, panelColor);
	
		if (!(window.Flags & InGuiWindowFlag::MenuEnabled))
		{			
			auto info = GenerateInGuiText(window.Mesh, renderConfig.Font, name, panelPos, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = info.Size.x + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, info.Size.y / 2 }, 4, info.Count * 4);
		}
		GenerateInGuiQuad(window.Mesh, closeButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::CLOSE_BUTTON]->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::MIN_BUTTON]->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		if (!(window.Flags & InGuiWindowFlag::Collapsed))
		{
			GenerateInGuiQuad(window.Mesh, winPos, winSize, renderConfig.SubTexture[InGuiRenderConfiguration::WINDOW]->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
			
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down left
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down right
								
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down right
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top right
									
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top right
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top left
									
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top left
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down left
		}
	}
	void InGuiFactory::GenerateRenderWindow(const char* name, InGuiWindow& window, uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.OverlayLineMesh.Vertices.clear();
		window.OverlayMesh.Vertices.clear();

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & InGuiWindowFlag::Moved)
			panelColor = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 closeButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };
		glm::vec2 minButtonPos = closeButtonPos - glm::vec2(InGuiWindow::PanelSize, 0.0f);
		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER]->GetTexCoords(), renderConfig.TextureID, panelColor);

		if (!(window.Flags & InGuiWindowFlag::MenuEnabled))
		{
			auto info = GenerateInGuiText(window.Mesh, renderConfig.Font, name, panelPos, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = info.Size.x + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, info.Size.y / 2 }, 4, info.Count * 4);
		}
		GenerateInGuiQuad(window.Mesh, closeButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::CLOSE_BUTTON]->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::MIN_BUTTON]->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });

		if (!(window.Flags & InGuiWindowFlag::Collapsed))
		{
			GenerateInGuiImage(window.Mesh, frameData.TexturePairs, rendererID, winPos, winSize, { 0,0,1,1 }, { 1,1,1,1 }, 1.0f);
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down left
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down right
									
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down right
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top right
								
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top right
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top left
									
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Top left
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] }); // Down left		
		}
	}
	
	void InGuiFactory::GenerateButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::BUTTON]->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto info = GenerateInGuiText(mesh, renderConfig.Font, name, {}, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - ((float)info.Size.y /2.0f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
	}
	void InGuiFactory::GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, bool value, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{	
		if (value)
			GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::CHECKBOX_CHECKED]->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::CHECKBOX_UNCHECKED]->GetTexCoords(), renderConfig.TextureID, color);

	}
	void InGuiFactory::GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name,float value, float scale, glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		char buffer[sc_MaxFloatValueBufferSize];
		snprintf(buffer, sizeof(buffer), "%f", value);
		buffer[sc_MaxFloatValueBufferSize - 1] = '\0';

		value *= scale;
		glm::vec2 handleSize = { size.y, size.y * 2 };		
		glm::vec2 handlePos = { position.x + value - handleSize.x / 2, position.y - (handleSize.x / 2) };
		
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER]->GetTexCoords(), renderConfig.TextureID, color);
		GenerateInGuiQuad(mesh, handlePos, handleSize, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER_HANDLE]->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		
		

		auto info = GenerateInGuiText(mesh, renderConfig.Font, buffer, {}, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - ((float)info.Size.y / 2.0f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
	}
	void InGuiFactory::GenerateImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const glm::vec4& texCoords, uint32_t rendererID, InGuiMesh& mesh,std::vector<TextureRendererIDPair>& texturePairs, const InGuiRenderConfiguration& renderConfig, float tilingFactor)
	{
		GenerateInGuiImage(mesh, texturePairs, rendererID, position, size, texCoords, color, tilingFactor);
	}
	void InGuiFactory::GenerateTextArea(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, const char* text,glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::BUTTON]->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto info = GenerateInGuiText(mesh, renderConfig.Font, text, {}, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - ((float)info.Size.y / 2.0f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);	
	}
	TextInfo InGuiFactory::GenerateText(const glm::vec4& color, const char* text, float length, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		return GenerateInGuiText(mesh, renderConfig.Font, text, {}, length, renderConfig.FontTextureID, color);
	}
	TextInfo InGuiFactory::GenerateText(const glm::vec4& color, const char* text, float length, InGuiVertex* vertices, const InGuiRenderConfiguration& renderConfig)
	{
		auto font = renderConfig.Font;
		TextInfo textInfo;
		float xCursor = 0.0f;
		float yCursor = 0.0f;

		uint32_t counter = 0;
		uint32_t i = 0;
		while (text[i] != '\0')
		{
			auto& character = font->GetCharacter(text[i]);
			glm::vec2 size = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};
			if (xCursor + size.x >= length)
				break;

			glm::vec2 offset = { character.XOffset, size.y - character.YOffset };
			glm::vec2 pos = { xCursor + offset.x, yCursor - offset.y };
			glm::vec4 coords = {
				(float)character.X0Coord / (float)font->GetWidth(), (float)character.Y0Coord / (float)font->GetHeight(),
				(float)character.X1Coord / (float)font->GetWidth(), (float)character.Y1Coord / (float)font->GetHeight()
			};
			vertices[counter++] = { color, { pos.x , pos.y, 0.0f },                  {coords.x, coords.w}, (float)renderConfig.FontTextureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y, 0.0f, },         {coords.z, coords.w}, (float)renderConfig.FontTextureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y + size.y, 0.0f }, {coords.z, coords.y}, (float)renderConfig.FontTextureID };
			vertices[counter++] = { color, { pos.x ,pos.y + size.y, 0.0f},           {coords.x, coords.y}, (float)renderConfig.FontTextureID };
			xCursor += character.XAdvance;
			textInfo.Size.x = xCursor;
			textInfo.Count++;
			if (textInfo.Size.y < size.y)
				textInfo.Size.y = size.y;
			i++;
		}
		return textInfo;
	}
	
	void InGuiFactory::GenerateColorPicker4(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		InGuiVertex vertices[4] = {
					   {{0,0,0,1}, {position.x,         position.y,0},		     {0,0},renderConfig.ColorPickerTextureID },
					   {{0,0,0,1}, {position.x + size.x,position.y,0},          {1,0}, renderConfig.ColorPickerTextureID },
					   { color,  {position.x + size.x,position.y + size.y,0}, {1,1},   renderConfig.ColorPickerTextureID },
					   {{1,1,1,1}, {position.x,         position.y + size.y,0}, {0,1}, renderConfig.ColorPickerTextureID }
		};
		GenerateInGuiQuad(mesh, vertices, 4);
	}

	void InGuiFactory::Generate6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		static constexpr uint32_t numSegments = 6;
		static constexpr uint32_t numVertices = numSegments * 4;
		float segmentSize = size.x / numSegments;

		size_t vertexOffset = mesh.Vertices.size();
		uint32_t counter = 0;

		float offset = 0.0f;
		while (counter < numVertices)
		{
			mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset, position.y, 0.0f },                 { 0,0 }, renderConfig.ColorPickerTextureID });
			mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize, position.y, 0.0f },   { 1,0 }, renderConfig.ColorPickerTextureID });
			mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize,position.y + size.y, 0.0f }, { 1,1 }, renderConfig.ColorPickerTextureID });
			mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset,position.y + size.y, 0.0f },               { 0,1 }, renderConfig.ColorPickerTextureID });

			offset += segmentSize;
			counter += 4;
		}

		mesh.Vertices[vertexOffset + 0].Color = { 1,0,0,1 };
		mesh.Vertices[vertexOffset + 1].Color = { 1,1,0,1 };
		mesh.Vertices[vertexOffset + 2].Color = { 1,1,0,1 };
		mesh.Vertices[vertexOffset + 3].Color = { 1,0,0,1 };
		/////////////////////
		mesh.Vertices[vertexOffset + 4].Color = { 1,1,0,1 };
		mesh.Vertices[vertexOffset + 5].Color = { 0,1,0,1 };
		mesh.Vertices[vertexOffset + 6].Color = { 0,1,0,1 };
		mesh.Vertices[vertexOffset + 7].Color = { 1,1,0,1 };
		/////////////////////
		mesh.Vertices[vertexOffset + 8].Color = { 0,1,0,1 };
		mesh.Vertices[vertexOffset + 9].Color = { 0,1,1,1 };
		mesh.Vertices[vertexOffset + 10].Color = { 0,1,1,1 };
		mesh.Vertices[vertexOffset + 11].Color = { 0,1,0,1 };
		/////////////////////
		mesh.Vertices[vertexOffset + 12].Color = { 0,1,1,1 };
		mesh.Vertices[vertexOffset + 13].Color = { 0,0,1,1 };
		mesh.Vertices[vertexOffset + 14].Color = { 0,0,1,1 };
		mesh.Vertices[vertexOffset + 15].Color = { 0,1,1,1 };
		/////////////////////
		mesh.Vertices[vertexOffset + 16].Color = { 0,0,1,1 };
		mesh.Vertices[vertexOffset + 17].Color = { 1,0,1,1 };
		mesh.Vertices[vertexOffset + 18].Color = { 1,0,1,1 };
		mesh.Vertices[vertexOffset + 19].Color = { 0,0,1,1 };
		/////////////////////
		mesh.Vertices[vertexOffset + 20].Color = { 1,0,1,1 };
		mesh.Vertices[vertexOffset + 21].Color = { 1,0,0,1 };
		mesh.Vertices[vertexOffset + 22].Color = { 1,0,0,1 };
		mesh.Vertices[vertexOffset + 23].Color = { 1,0,1,1 };
	}

	void InGuiFactory::GenerateGroup(const glm::vec2& position, const glm::vec4& color, const const char* name,bool open,InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		glm::vec2 minButtonPos = { position.x + 5, position.y };
		GenerateInGuiQuad(window->Mesh, position, { window->Size.x ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER]->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto info = GenerateInGuiText(window->Mesh, renderConfig.Font, name, { minButtonPos.x + InGuiWindow::PanelSize, position.y }, window->Size.x, renderConfig.FontTextureID, color);
		glm::vec2 textOffset = { 5.0f, (info.Size.y / 1.5f) };
		MoveVertices(window->Mesh.Vertices.data(), textOffset, offset, info.Count * 4);

		if (open)
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::DOWN_ARROW]->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::RIGHT_ARROW]->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateQuad(const glm::vec2& position,const glm::vec2& size, const glm::vec4& color, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::BUTTON]->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateMenuBar(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::BUTTON]->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto info = GenerateInGuiText(mesh, renderConfig.Font, name, {}, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - (info.Size.y / 1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
	}

	void InGuiFactory::GenerateFrame(InGuiLineMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		mesh.Vertices.push_back({ { position.x,position.y,0 }, color }); // Down left
		mesh.Vertices.push_back({ { position.x + size.x, position.y,0 }, color }); // Down right

		mesh.Vertices.push_back({ { position.x + size.x,position.y,0 }, color }); // Down right
		mesh.Vertices.push_back({ { position.x + size.x,position.y + size.y,0 }, color }); // Top right


		mesh.Vertices.push_back({ { position.x + size.x, position.y + size.y,0 }, color }); // Top right
		mesh.Vertices.push_back({ { position.x, position.y + size.y,0 }, color }); // Top left

		mesh.Vertices.push_back({ { position.x,position.y + size.y,0 }, color }); // Top left
		mesh.Vertices.push_back({ { position.x,position.y,0 }, color }); // Down left
	}

	void InGuiFactory::GenerateArrowLine(InGuiMesh& mesh, InGuiLineMesh& lineMesh, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		lineMesh.Vertices.push_back({ { p0.x, p0.y, 0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] });
		lineMesh.Vertices.push_back({ { p1.x, p1.y, 0 }, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR] });


		glm::vec4 half = glm::vec4(p1.x - ((p1.x - p0.x) / 2), p1.y - ((p1.y - p0.y) / 2), 0, 0);


		size_t offset = mesh.Vertices.size();
		glm::vec2 dir = p1 - p0;
		glm::vec2 defaultV = { 0,-1 };

		float dot = dir.x * defaultV.x + dir.y * defaultV.y;
		float det = dir.x * defaultV.y - dir.y * defaultV.x;
		float angle = atan2(det, dot);
		

		glm::vec2 arrowOffset = { -cos(angle) * (size.x / 2), -sin(angle) * (size.y / 2) };
		glm::mat4 translation = glm::translate(glm::vec3{ half.x + arrowOffset.x, half.y - arrowOffset.y ,0 });
		glm::mat4 rotation = glm::rotate(-angle, glm::vec3{ 0,0,1 });
	
		GenerateInGuiQuad(mesh, { 0,0 }, size, renderConfig.SubTexture[InGuiRenderConfiguration::DOWN_ARROW]->GetTexCoords(), renderConfig.TextureID, renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR]);
		for (size_t i = offset; i < mesh.Vertices.size(); ++i)
			mesh.Vertices[i].Position = translation * rotation * glm::vec4(mesh.Vertices[i].Position, 1);
	}

	void InGuiFactory::GenerateIcon(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size,const glm::vec4& color, const Ref<SubTexture2D>& subTexture, uint32_t textureID)
	{
		GenerateInGuiQuad(mesh, position, size, subTexture->GetTexCoords(), textureID, color);
	}

	void InGuiFactory::GenerateNode(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{	
		glm::vec2 panelPos = { position.x, position.y + size.y };

		GenerateInGuiQuad(mesh, panelPos, { size.x ,InGuiWindow::PanelSize }, renderConfig.SubTexture[InGuiRenderConfiguration::SLIDER]->GetTexCoords(), renderConfig.TextureID, color);
		
		size_t offset = mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(mesh, renderConfig.Font, name, panelPos,  size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		MoveVertices(mesh.Vertices.data(), { 5, height / 2 }, offset, strlen(name) * 4);
		
		GenerateInGuiQuad(mesh, position, size, renderConfig.SubTexture[InGuiRenderConfiguration::WINDOW]->GetTexCoords(), renderConfig.TextureID, color);
	}

}