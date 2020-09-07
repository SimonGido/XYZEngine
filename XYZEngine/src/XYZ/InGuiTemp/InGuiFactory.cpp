#include "stdafx.h"
#include "InGuiFactory.h"


namespace XYZ {
	static void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
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
	static void GenerateInGuiImage(InGuiMesh& mesh, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color, InGuiRenderConfiguration& renderConfig)
	{
		uint32_t textureID = 0;
		for (auto& pair : mesh.TexturePairs)
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
		mesh.TexturePairs.push_back({ textureID,rendererID });
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
		window.Mesh.TexturePairs.clear();
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
			GenerateInGuiQuad(window.Mesh, winPos, winSize, renderConfig.WindowSubTexture->GetTexCoords(), renderConfig.TextureID, { 1,1,1,1 });
		
	}
	void InGuiFactory::GenerateRenderWindow(InGuiWindow& window, uint32_t rendererID, InGuiRenderConfiguration& renderConfig)
	{
		size_t lastFrameSize = window.Mesh.Vertices.size();
		window.Mesh.TexturePairs.clear();
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
			GenerateInGuiImage(window.Mesh, rendererID, winPos, winSize, { 0,0,1,1 }, { 1,1,1,1 }, renderConfig);
	}
	void InGuiFactory::GenerateButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.ButtonSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { (size.x / 2) - (width / 2),(size.y / 2.0f) - ((float)height/2.0f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
	}
	void InGuiFactory::GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, bool value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		if (value)
			GenerateInGuiQuad(window->Mesh, position, size, renderConfig.CheckboxSubTextureChecked->GetTexCoords(), renderConfig.TextureID, color);
		else
			GenerateInGuiQuad(window->Mesh, position, size, renderConfig.CheckboxSubTextureUnChecked->GetTexCoords(), renderConfig.TextureID, color);
		
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { size.x + 5 ,(size.y / 2.0f) - ((float)height / 2.0f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);

		frameData.WindowSpaceOffset.x += width + 5;
	}
	void InGuiFactory::GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name,float value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig)
	{
		auto window = frameData.CurrentWindow;
		glm::vec2 handleSize = { size.y, size.y * 2 };
		size_t offset = window->Mesh.Vertices.size();
		auto [width, height] = GenerateInGuiText(window->Mesh, renderConfig.Font, name, {}, { 0.7,0.7 }, size.x, renderConfig.FontTextureID, { 1,1,1,1 });
		glm::vec2 textOffset = { size.x + 5 ,(size.y / 2.0f) - ((float)height / 2.0f) };
		MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);
		frameData.WindowSpaceOffset.x += width + 5;

		glm::vec2 handlePos = { position.x + value - handleSize.x / 2, position.y - (handleSize.x / 2) };
		GenerateInGuiQuad(window->Mesh, position, size, renderConfig.SliderSubTexture->GetTexCoords(), renderConfig.TextureID, color);
		GenerateInGuiQuad(window->Mesh, handlePos, handleSize, renderConfig.SliderHandleSubTexture->GetTexCoords(), renderConfig.TextureID, color);
	}
}