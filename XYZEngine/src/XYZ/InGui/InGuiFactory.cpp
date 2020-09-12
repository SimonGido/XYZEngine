#include "stdafx.h"
#include "InGuiFactory.h"


namespace XYZ {
	static void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const uint32_t textureID, const glm::vec4& color)
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
			mesh.Vertices.push_back({ color, quadVertexPositions[i], texCoords[i], textureID });
		}
	}

	static void GenerateInGuiQuad(InGuiMesh& mesh, InGuiVertex* vertices, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			mesh.Vertices.push_back(vertices[i]);
	}

	static void GenerateInGuiImage(InGuiMesh& mesh,InGuiPerFrameData& frameData, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, const InGuiRenderConfiguration& renderConfig)
	{
		uint32_t textureID = 0;
		for (auto& pair : frameData.TexturePairs)
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
		GenerateInGuiQuad(mesh, position, size, texCoord, textureID, color);
		frameData.TexturePairs.push_back({ textureID,rendererID });
	}

	


	static std::pair<int32_t, int32_t> GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
	{
		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;

		int32_t width = 0;
		int32_t height = 0;

		for (auto c : str)
		{
			auto& character = font->GetCharacter(c);
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

			mesh.Vertices.push_back({ color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,textureID });
			mesh.Vertices.push_back({ color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,textureID });
			mesh.Vertices.push_back({ color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,textureID });

			if (size.y > height)
				height = size.y;


			width += character.XAdvance * scale.x;
			cursorX += character.XAdvance * scale.x;
		}
		return std::pair<int32_t, int32_t>(width, height);
	}
	static std::pair<int32_t, int32_t> GenerateInGuiText(InGuiVertex* vertices, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color)
	{
		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;

		int32_t width = 0;
		int32_t height = 0;
		uint32_t counter = 0;
		for (auto c : str)
		{
			auto& character = font->GetCharacter(c);
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

			vertices[counter++] = { color, { pos.x , pos.y, 0.0f }, coords / scaleFont ,textureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y, 0.0f, }, (coords + glm::vec2(character.Width, 0)) / scaleFont,textureID };
			vertices[counter++] = { color, { pos.x + size.x, pos.y + size.y, 0.0f }, (coords + glm::vec2(character.Width, character.Height)) / scaleFont,textureID };
			vertices[counter++] = { color, { pos.x ,pos.y + size.y, 0.0f}, (coords + glm::vec2(0,character.Height)) / scaleFont,textureID };

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

	void InGuiFactory::GenerateWindow(InGuiWindow& window, const InGuiRenderConfiguration& renderConfig)
	{
		size_t lastFrameSize = window.Mesh.Vertices.size();
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.Mesh.Vertices.reserve(lastFrameSize);

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & Moved)
			panelColor = renderConfig.HooverColor;

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, panelColor);
	
		if (!(window.Flags & MenuEnabled))
		{
			auto [width, height] = GenerateInGuiText(window.Mesh, renderConfig.Font, window.Name, panelPos, { 0.7f,0.7f }, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = width + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, height / 2 }, 4, window.Name.size() * 4);
		}
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.MinimizeButtonSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		if (!(window.Flags & Collapsed))
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
	void InGuiFactory::GenerateRenderWindow(InGuiWindow& window, uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		size_t lastFrameSize = window.Mesh.Vertices.size();
		window.LineMesh.Vertices.clear();
		window.Mesh.Vertices.clear();
		window.Mesh.Vertices.reserve(lastFrameSize);

		glm::vec4 panelColor = { 1,1,1,1 };
		if (window.Flags & Moved)
			panelColor = renderConfig.HooverColor;

		glm::vec2 winPos = window.Position;
		glm::vec2 winSize = window.Size;
		glm::vec2 panelPos = { winPos.x, winPos.y + winSize.y };
		glm::vec2 minButtonPos = { panelPos.x + winSize.x - InGuiWindow::PanelSize, panelPos.y };

		GenerateInGuiQuad(window.Mesh, panelPos, { winSize.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, panelColor);

		if (!(window.Flags & MenuEnabled))
		{
			auto [width, height] = GenerateInGuiText(window.Mesh, renderConfig.Font, window.Name, panelPos, { 0.7f,0.7f }, window.Size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			window.MinimalWidth = width + InGuiWindow::PanelSize;
			MoveVertices(window.Mesh.Vertices.data(), { 5, height / 2 }, 4, window.Name.size() * 4);
		}
		GenerateInGuiQuad(window.Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.MinimizeButtonSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		if (!(window.Flags & Collapsed))
		{
			GenerateInGuiImage(window.Mesh, frameData,rendererID, winPos, winSize, { 0,0,1,1 }, { 1,1,1,1 }, renderConfig);
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
	void InGuiFactory::GenerateButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2.0f) - ((float)height/1.5f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
	}
	void InGuiFactory::GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, bool value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		
		if (value)
			GenerateInGuiQuad(window->Mesh, position, size, renderConfig.CheckboxSubTextureChecked->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(window->Mesh, position, size, renderConfig.CheckboxSubTextureUnChecked->GetTexCoords(), renderConfig.TextureID, color);

	}
	void InGuiFactory::GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name,float value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		glm::vec2 handleSize = { size.y, size.y * 2 };
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { size.x + 5 ,(size.y / 2.0f) - ((float)height / 1.5f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
		frameData.WindowSpaceOffset.x += width + 5;

		glm::vec2 handlePos = { position.x + value - handleSize.x / 2, position.y - (handleSize.x / 2) };
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		GenerateInGuiQuad(window->Mesh, handlePos, handleSize, renderConfig.SliderHandleSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}
	void InGuiFactory::GenerateImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiImage(window->Mesh,frameData, rendererID, position,size, { 0,0,1,1 }, color, renderConfig);
	}
	void InGuiFactory::GenerateTextArea(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, const std::string& text, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		{		
			size_t offset = window->Mesh.Vertices.size();
			auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7f,0.7f }, 1000.0f, renderConfig.FontTextureID, { 1,1,1,1 });
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - ((float)height / 1.5f) };
			MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
			frameData.WindowSpaceOffset.x += width + 5;
		}
		{
			GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
			size_t offset = window->Mesh.Vertices.size();
			auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, text, {}, { 0.7f,0.7f }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
			glm::vec2 textOffset = { (size.x / 2) - (width / 2),((float)height / 1.5f) };
			MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, text.size() * 4);
		}
	}
	std::pair<int32_t, int32_t> InGuiFactory::GenerateText(const glm::vec2& scale, const glm::vec4& color, const std::string& text, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		return GenerateInGuiText(window->Mesh, renderConfig.Font, text, {}, scale, 100.0f, renderConfig.FontTextureID, color);
	}
	void InGuiFactory::GenerateColorPicker4(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		InGuiVertex vertices[4] = {
					   {{0,0,0,1}, {position.x,         position.y,0},		     {0,0},renderConfig.ColorPickerTextureID },
					   {{0,0,0,1}, {position.x + size.x,position.y,0},          {1,0}, renderConfig.ColorPickerTextureID },
					   { color,  {position.x + size.x,position.y + size.y,0}, {1,1},   renderConfig.ColorPickerTextureID },
					   {{1,1,1,1}, {position.x,         position.y + size.y,0}, {0,1}, renderConfig.ColorPickerTextureID }
		};
		GenerateInGuiQuad(window->Mesh, vertices, 4);
	}

	void InGuiFactory::Generate6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		static constexpr uint32_t numSegments = 6;
		static constexpr uint32_t numVertices = numSegments * 4;
		float segmentSize = size.x / numSegments;

		size_t vertexOffset = window->Mesh.Vertices.size();
		uint32_t counter = 0;

		float offset = 0.0f;
		while (counter < numVertices)
		{
			window->Mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset, position.y, 0.0f },                 { 0,0 }, renderConfig.ColorPickerTextureID });
			window->Mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize, position.y, 0.0f },   { 1,0 }, renderConfig.ColorPickerTextureID });
			window->Mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset + segmentSize,position.y + size.y, 0.0f }, { 1,1 }, renderConfig.ColorPickerTextureID });
			window->Mesh.Vertices.push_back({ { 1,1,1,1 }, {position.x + offset,position.y + size.y, 0.0f },               { 0,1 }, renderConfig.ColorPickerTextureID });

			offset += segmentSize;
			counter += 4;
		}

		window->Mesh.Vertices[vertexOffset + 0].Color = { 1,0,0,1 };
		window->Mesh.Vertices[vertexOffset + 1].Color = { 1,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 2].Color = { 1,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 3].Color = { 1,0,0,1 };
		//////////////////////////////
		window->Mesh.Vertices[vertexOffset + 4].Color = { 1,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 5].Color = { 0,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 6].Color = { 0,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 7].Color = { 1,1,0,1 };
		//////////////////////////////
		window->Mesh.Vertices[vertexOffset + 8].Color = { 0,1,0,1 };
		window->Mesh.Vertices[vertexOffset + 9].Color = { 0,1,1,1 };
		window->Mesh.Vertices[vertexOffset + 10].Color = { 0,1,1,1 };
		window->Mesh.Vertices[vertexOffset + 11].Color = { 0,1,0,1 };
		//////////////////////////////
		window->Mesh.Vertices[vertexOffset + 12].Color = { 0,1,1,1 };
		window->Mesh.Vertices[vertexOffset + 13].Color = { 0,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 14].Color = { 0,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 15].Color = { 0,1,1,1 };
		//////////////////////////////
		window->Mesh.Vertices[vertexOffset + 16].Color = { 0,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 17].Color = { 1,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 18].Color = { 1,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 19].Color = { 0,0,1,1 };
		//////////////////////////////
		window->Mesh.Vertices[vertexOffset + 20].Color = { 1,0,1,1 };
		window->Mesh.Vertices[vertexOffset + 21].Color = { 1,0,0,1 };
		window->Mesh.Vertices[vertexOffset + 22].Color = { 1,0,0,1 };
		window->Mesh.Vertices[vertexOffset + 23].Color = { 1,0,1,1 };
	}

	void InGuiFactory::GenerateGroup(const glm::vec2& position, const glm::vec4& color, const std::string& name,bool open,InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		glm::vec2 minButtonPos = { position.x + 5, position.y };
		GenerateInGuiQuad(window->Mesh, position, { window->Size.x ,InGuiWindow::PanelSize }, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, { minButtonPos.x + InGuiWindow::PanelSize, position.y }, { 0.7f,0.7f }, window->Size.x, renderConfig.FontTextureID, color);
		MoveVertices(window->Mesh.Vertices.data(), { 5, height / 2 }, offset, name.size() * 4);

		if (open)
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.DownArrowButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(window->Mesh, minButtonPos, { InGuiWindow::PanelSize ,InGuiWindow::PanelSize }, renderConfig.RightArrowButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateQuad(const glm::vec2& position,const glm::vec2& size, const glm::vec4& color, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}

	void InGuiFactory::GenerateMenuBar(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2.0f) - ((float)height / 1.5f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
	}

	void InGuiFactory::GenerateTestButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig)
	{
		GenerateInGuiQuad(mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2.0f) - ((float)height / 1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
	}

	
}