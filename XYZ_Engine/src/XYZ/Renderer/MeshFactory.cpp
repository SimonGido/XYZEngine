#include "stdafx.h"
#include "MeshFactory.h"


namespace XYZ {

	Ref<Mesh> MeshFactory::CreateTextMesh(const TextUI& text, const glm::vec4& color)
	{
		auto& fontData = text.Font->GetData();
		Ref<Mesh> mesh = Ref<Mesh>::Create();
		int32_t cursorX = 0, cursorY = 0;

		for (auto c : text.Text)
		{
			auto& character = text.Font->GetCharacter(c);
			glm::vec2 position = {
				cursorX + character.XOffset,
				cursorY
			};

			glm::vec2 size =   { character.Width, character.Height };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scale =  { fontData.ScaleW, fontData.ScaleH };

			Vertex vertices[4] = {
				{ { position.x ,         position.y,         0.0f, 1.0f }, color, coords / scale},
				{ { position.x + size.x, position.y,          0.0f, 1.0f }, color, (coords + glm::vec2(character.Width,                0)) / scale},
				{ { position.x + size.x, position.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, character.Height)) / scale},
				{ { position.x ,         position.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(0,               character.Height)) / scale}
			};

			cursorX += character.XAdvance;

			for (auto& vertex : vertices)
				mesh->Vertices.push_back(vertex);
		}

		mesh->Indices.resize(text.Text.size() * 6);

		return mesh;
	}

	Ref<Mesh> MeshFactory::CreateSprite(const glm::vec4& color, const glm::vec4& texCoord,int32_t textureID)
	{
		Ref<Mesh> mesh = Ref<Mesh>::Create();
		mesh->TextureID = textureID;
		Vertex vertices[4];
		vertices[0].Position = {- (1.0f / 2.0f),- (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[1].Position = {  (1.0f / 2.0f),- (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[2].Position = {  (1.0f / 2.0f),  (1.0f / 2.0f), 0.0f, 1.0f };
		vertices[3].Position = {- (1.0f / 2.0f),  (1.0f / 2.0f), 0.0f, 1.0f };

		vertices[0].Color = color;
		vertices[1].Color = color;
		vertices[2].Color = color;
		vertices[3].Color = color;

		vertices[0].TexCoord = { texCoord.x,texCoord.y };
		vertices[1].TexCoord = { texCoord.z,texCoord.y };
		vertices[2].TexCoord = { texCoord.z,texCoord.w };
		vertices[3].TexCoord = { texCoord.x,texCoord.w };

		for (auto& vertex : vertices)
			mesh->Vertices.push_back(vertex);
		
		mesh->Indices.resize(6);

		return mesh;
	}
}