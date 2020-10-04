#include "stdafx.h"
#include "InGuiFactory.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {
	
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

	static void GenerateInGuiImage(InGuiMesh& mesh,std::vector<TextureRendererIDPair>& texturePairs, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, const InGuiRenderConfiguration& renderConfig, float tilingFactor)
	{
		uint32_t textureID = 0;
		for (auto& pair : texturePairs)
		{
			if (pair.RendererID == rendererID)
			{
				textureID = pair.TextureID;
			}
		}
		if (!textureID)
		{
			textureID = renderConfig.NumTexturesInUse;
			renderConfig.NumTexturesInUse++;
		}
		GenerateInGuiQuad(mesh, position, size, texCoord, textureID, color, tilingFactor);
		texturePairs.push_back({ textureID,rendererID });
	}

	


	static TextInfo GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const char* str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
	{
		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;

		TextInfo textInfo;
		for (size_t i = 0; i < strlen(str); ++i)
		{
			auto& character = font->GetCharacter(str[i]);
			if (textInfo.Size.x + (character.XAdvance * scale.x) >= length)
				break;

			float yOffset = (fontData.LineHeight - character.YOffset - character.Height) * scale.y;
			glm::vec2 pos = {
				cursorX + character.XOffset + position.x,
				cursorY + yOffset + position.y
			};

			glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

			mesh.Vertices.push_back({ color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,(float)textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,(float)textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,(float)textureID });
			mesh.Vertices.push_back({ color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,(float)textureID });

			if (size.y > textInfo.Size.y)
				textInfo.Size.y = size.y;


			textInfo.Size.x += character.XAdvance * scale.x;
			textInfo.Count++;
			cursorX += character.XAdvance * scale.x;
		}
		return textInfo;
	}
	static std::pair<int32_t, int32_t> GenerateInGuiText(InGuiVertex* vertices, const Ref<Font>& font, const char* str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
	{
		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;

		int32_t width = 0;
		int32_t height = 0;
		uint32_t counter = 0;
		for (size_t i = 0; i < strlen(str); ++i)
		{
			auto& character = font->GetCharacter(str[i]);
			if (width + (character.XAdvance * scale.x) >= length)
				break;

			float yOffset = (fontData.LineHeight - character.YOffset - character.Height) * scale.y;
			glm::vec2 pos = {
				cursorX + character.XOffset + position.x,
				cursorY + yOffset + position.y
			};

			glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

			vertices[counter++] = { color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,(float)textureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,(float)textureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,(float)textureID };
			vertices[counter++] = { color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,(float)textureID };

			if (size.y > height)
				height = size.y;


			width += character.XAdvance * scale.x;
			cursorX += character.XAdvance * scale.x;
		}

		return std::pair<int32_t, int32_t>(width, height);
	}
	static void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, size_t offset, size_t count)
	{
		for (size_t i = offset; i < count + offset; ++i)
		{
			vertices[i].Position.x += position.x;
			vertices[i].Position.y += position.y;
		}
	}

	void InGuiFactory::GenerateWindow(const char* name,InGuiWindow& window, const InGuiRenderConfiguration& renderConfig)
	{
		size_t lastFrameSize = window.Mesh.Vertices.size();
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.Mesh.Vertices.reserve(lastFrameSize);

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & InGuiWindowFlag::Moved)
			panelColor = renderConfig.HooverColor;

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, panelColor);
	
		if (!(window.Flags & InGuiWindowFlag::MenuEnabled))
		{
			auto info = GenerateInGuiText(window.Mesh, renderConfig.Font, name, panelPos, { 0.7f,0.7f }, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = info.Size.x + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, info.Size.y / 2 }, 4, info.Count * 4);
		}
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.MinimizeButtonSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		if (!(window.Flags & InGuiWindowFlag::Collapsed))
		{
			GenerateInGuiQuad(window.Mesh, winPos, winSize, renderConfig.WindowSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
			
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.LineColor }); // Down left
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.LineColor }); // Down right
								
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.LineColor }); // Down right
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top right
									
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top right
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top left
									
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top left
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.LineColor }); // Down left
		}
	}
	void InGuiFactory::GenerateRenderWindow(const char* name, InGuiWindow& window, uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		size_t lastFrameSize = window.Mesh.Vertices.size();
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.Mesh.Vertices.reserve(lastFrameSize);

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & InGuiWindowFlag::Moved)
			panelColor = renderConfig.HooverColor;

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, panelColor);

		if (!(window.Flags & InGuiWindowFlag::MenuEnabled))
		{
			auto info = GenerateInGuiText(window.Mesh, renderConfig.Font, name, panelPos, { 0.7f,0.7f }, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = info.Size.x + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, info.Size.y / 2 }, 4, info.Count * 4);
		}
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.MinimizeButtonSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		if (!(window.Flags & InGuiWindowFlag::Collapsed))
		{
			GenerateInGuiImage(window.Mesh, frameData.TexturePairs, rendererID, winPos, winSize, { 0,0,1,1 }, { 1,1,1,1 }, renderConfig, 1.0f);
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.LineColor }); // Down left
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.LineColor }); // Down right
									
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y,0 }, renderConfig.LineColor }); // Down right
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top right
								
			window.LineMesh.Vertices.push_back({ { window.Position.x + window.Size.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top right
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top left
									
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y + window.Size.y,0 }, renderConfig.LineColor }); // Top left
			window.LineMesh.Vertices.push_back({ { window.Position.x,window.Position.y,0 }, renderConfig.LineColor }); // Down left		
		}
	}
	
	void InGuiFactory::GenerateButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto info = GenerateInGuiText(mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - ((float)info.Size.y /1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
	}
	void InGuiFactory::GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, bool value, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{	
		if (value)
			GenerateInGuiQuad(mesh, position, size, renderConfig.CheckboxSubTextureChecked->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(mesh, position, size, renderConfig.CheckboxSubTextureUnChecked->GetTexCoords(), renderConfig.TextureID, color);

	}
	void InGuiFactory::GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name,float value, glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		glm::vec2 handleSize = { size.y, size.y * 2 };
		size_t offset = mesh.Vertices.size();
		auto info = GenerateInGuiText(mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { size.x + 5 ,(size.y / 2.0f) - ((float)info.Size.y / 1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
		windowSpaceOffset.x += info.Size.x + 5;
		
		glm::vec2 handlePos = { position.x + value - handleSize.x / 2, position.y - (handleSize.x / 2) };
		GenerateInGuiQuad(mesh, position, size, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		GenerateInGuiQuad(mesh, handlePos, handleSize, renderConfig.SliderHandleSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}
	void InGuiFactory::GenerateImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t rendererID, InGuiMesh& mesh,std::vector<TextureRendererIDPair>& texturePairs, const InGuiRenderConfiguration& renderConfig, float tilingFactor)
	{
		GenerateInGuiImage(mesh, texturePairs, rendererID, position, size, { 0,0,1,1 }, color, renderConfig, tilingFactor);
	}
	void InGuiFactory::GenerateTextArea(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, const char* text,glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		{		
			size_t offset = mesh.Vertices.size();
			auto info = GenerateInGuiText(mesh, renderConfig.Font, name, {}, { 0.7f,0.7f }, 1000.0f, renderConfig.FontTextureID, { 1,1,1,1 });
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - ((float)info.Size.y / 1.5f) };
			MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
			windowSpaceOffset.x += info.Size.x + 5;
		}
		{
			GenerateInGuiQuad(mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
			size_t offset = mesh.Vertices.size();
			auto info = GenerateInGuiText(mesh, renderConfig.Font, text, {}, { 0.7f,0.7f }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(info.Size.y / 1.5f) };
			MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
		}
	}
	TextInfo InGuiFactory::GenerateText(const glm::vec2& scale, const glm::vec4& color, const char* text, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		return GenerateInGuiText(mesh, renderConfig.Font, text, {}, scale, 1000.0f, renderConfig.FontTextureID, color);
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
		GenerateInGuiQuad(window->Mesh, position, { window->Size.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, { minButtonPos.x + InGuiWindow::PanelSize, position.y }, { 0.7f,0.7f }, window->Size.x, renderConfig.FontTextureID, color);
		MoveVertices(window->Mesh.Vertices.data(), { 5, height / 2 }, offset, strlen(name) * 4);

		if (open)
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.DownArrowButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.RightArrowButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateQuad(const glm::vec2& position,const glm::vec2& size, const glm::vec4& color, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateMenuBar(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto info = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (info.Size.x / 2),(size.y / 2.0f) - (info.Size.y / 1.5f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
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
		lineMesh.Vertices.push_back({ { p0.x, p0.y, 0 }, renderConfig.LineColor });
		lineMesh.Vertices.push_back({ { p1.x, p1.y, 0 }, renderConfig.LineColor });


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
	
		GenerateInGuiQuad(mesh, { 0,0 }, size, renderConfig.DownArrowButtonSubTexture->GetTexCoords(), renderConfig.TextureID, renderConfig.DefaultColor);
		for (size_t i = offset; i < mesh.Vertices.size(); ++i)
			mesh.Vertices[i].Position = translation * rotation * glm::vec4(mesh.Vertices[i].Position, 1);
	}

	void InGuiFactory::GenerateNode(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const glm::vec4& panelColor, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{	
		glm::vec2 panelPos = { position.x, position.y + size.y };

		GenerateInGuiQuad(mesh, panelPos, { size.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, panelColor);
		
		size_t offset = mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(mesh, renderConfig.Font, name, panelPos, { 0.7f,0.7f }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		MoveVertices(mesh.Vertices.data(), { 5, height / 2 }, offset, strlen(name) * 4);
		
		GenerateInGuiQuad(mesh, position, size, renderConfig.WindowSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}

	
}