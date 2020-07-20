#include "stdafx.h"
#include "InGuiRenderer.h"

#include "VertexArray.h"
#include "RenderCommand.h"
#include "XYZ/Renderer/Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace XYZ {

	struct Vertex2D
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
		float	  TextureID;
	};

	struct TexturePair
	{
		uint32_t RendererID;
		uint32_t Slot;
	};

	struct RendererUIData
	{
		void Reset();

		Ref<Material> Material;

		static const uint32_t MaxTextures = 32;
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		std::vector<TexturePair> Textures;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		uint32_t IndexCount = 0;
		Vertex2D* BufferBase = nullptr;
		Vertex2D* BufferPtr = nullptr;

		UIRenderData Data;
	};
	static RendererUIData s_UIData;

	void InGuiRenderer::Init()
	{
		s_UIData.Reset();
	}
	void InGuiRenderer::Shutdown()
	{
	}
	void InGuiRenderer::BeginScene(const UIRenderData& data)
	{
		s_UIData.Data = data;
	}
	void InGuiRenderer::SetMaterial(const Ref<Material>& material)
	{
		if (s_UIData.Material && material.Raw() != s_UIData.Material.Raw())
			Flush();

		s_UIData.Material = material;
	}
	void InGuiRenderer::SubmitUI(const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_UIData.IndexCount + 6 > s_UIData.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec4 quadVertexPositions[4] = {
			{ position.x - (size.x / 2.0f),position.y - (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x + (size.x / 2.0f),position.y - (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x + (size.x / 2.0f),position.y + (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x - (size.x / 2.0f),position.y + (size.y / 2.0f), 0.0f, 1.0f }
		};

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_UIData.BufferPtr->Position = quadVertexPositions[i];
			s_UIData.BufferPtr->Color = color;
			s_UIData.BufferPtr->TexCoord = texCoords[i];
			s_UIData.BufferPtr->TextureID = (float)textureID;
			s_UIData.BufferPtr++;
		}
		s_UIData.IndexCount += 6;
	}

	void InGuiRenderer::SubmitUI(uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color)
	{	
		constexpr size_t quadVertexCount = 4;
		if (s_UIData.IndexCount + 6 > s_UIData.MaxIndices ||
			s_UIData.Textures.size() + s_UIData.Material->GetNumberOfTextures() >= s_UIData.MaxTextures)
			Flush();

		uint32_t textureID = 0;
		for (uint32_t i = 0; i < s_UIData.Textures.size(); ++i)
		{
			if (s_UIData.Textures[i].RendererID == rendererID)
			{
				textureID = s_UIData.Textures[i].Slot;
				break;
			}
		}
		if (!textureID)
		{
			textureID += s_UIData.Material->GetNumberOfTextures();
			s_UIData.Textures.push_back({ rendererID, textureID });
		}

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec4 quadVertexPositions[4] = {
			{ position.x - (size.x / 2.0f),position.y - (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x + (size.x / 2.0f),position.y - (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x + (size.x / 2.0f),position.y + (size.y / 2.0f), 0.0f, 1.0f },
			{ position.x - (size.x / 2.0f),position.y + (size.y / 2.0f), 0.0f, 1.0f }
		};

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_UIData.BufferPtr->Position = quadVertexPositions[i];
			s_UIData.BufferPtr->Color = color;
			s_UIData.BufferPtr->TexCoord = texCoords[i];
			s_UIData.BufferPtr->TextureID = (float)textureID;
			s_UIData.BufferPtr++;
		}
		s_UIData.IndexCount += 6;
	}

	void InGuiRenderer::SubmitText(const std::string& text,const Ref<Font>& font, const glm::vec2& position, const glm::vec2& scale, uint32_t textureID, const glm::vec4& color)
	{
		if (s_UIData.IndexCount + (text.size() * 6) > s_UIData.MaxIndices)
			Flush();

		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;
		
		for (auto c : text)
		{
			auto& character = font->GetCharacter(c);
			glm::vec2 pos = {
				cursorX + character.XOffset + position.x,
				cursorY + position.y
			};


			glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

			Vertex vertices[4] = {
				{ { pos.x ,         pos.y,          0.0f, 1.0f }, color,  coords / scaleFont},
				{ { pos.x + size.x, pos.y,          0.0f, 1.0f }, color, (coords + glm::vec2(character.Width,                0)) / scaleFont},
				{ { pos.x + size.x, pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, character.Height)) / scaleFont},
				{ { pos.x ,         pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(0,               character.Height)) / scaleFont}
			};

			cursorX += character.XAdvance * scale.x;
			for (auto& vertex : vertices)
			{
				s_UIData.BufferPtr->Position = vertex.Position;
				s_UIData.BufferPtr->Color = color;
				s_UIData.BufferPtr->TexCoord = vertex.TexCoord;
				s_UIData.BufferPtr->TextureID = (float)textureID;
				s_UIData.BufferPtr++;
			}
			s_UIData.IndexCount += 6;
		}
	}

	void InGuiRenderer::SubmitCenteredText(const std::string& text, const Ref<Font>& font, const glm::vec2& position, const glm::vec2& scale, uint32_t textureID, const glm::vec4& color, int centered)
	{
		if (s_UIData.IndexCount + (text.size() * 6) > s_UIData.MaxIndices)
			Flush();

		auto& fontData = font->GetData();
		int32_t cursorX = 0, cursorY = 0;
		int32_t width = 0;
		int32_t height = 0;

		std::vector<Vertex> vertices;
		vertices.reserve(text.size() * 4);
		for (auto c : text)
		{
			auto& character = font->GetCharacter(c);
			glm::vec2 pos = {
				cursorX + character.XOffset + position.x,
				cursorY + position.y
			};

			glm::vec2 size = { character.Width * scale.x, character.Height * scale.y };
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scaleFont = { fontData.ScaleW, fontData.ScaleH };

			vertices.push_back({ { pos.x , pos.y, 0.0f, 1.0f }, color,  coords / scaleFont });
			vertices.push_back({ { pos.x + size.x, pos.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, 0)) / scaleFont });
			vertices.push_back({ { pos.x + size.x, pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(character.Width, character.Height)) / scaleFont });
			vertices.push_back({ { pos.x ,pos.y + size.y, 0.0f, 1.0f }, color, (coords + glm::vec2(0,character.Height)) / scaleFont });

			if (size.y > height)
				height = size.y;

			width += character.XAdvance * scale.x;
			cursorX += character.XAdvance * scale.x;
		}

		if (centered == (Middle | Top))
		{
			width = -width / 2;
		}
		else if (centered == (Middle | Bottom))
		{
			
		}
		else if (centered == Middle)
		{
			width = -width / 2;
			height = -height / 2;
		}
		else if (centered == (Middle | Right))
		{
			height = -height / 2;
			width = 0;
		}

		for (auto& vertex : vertices)
		{
			vertex.Position.x += width;
			vertex.Position.y += height;
			s_UIData.BufferPtr->Position = vertex.Position;
			s_UIData.BufferPtr->Color = color;
			s_UIData.BufferPtr->TexCoord = vertex.TexCoord;
			s_UIData.BufferPtr->TextureID = (float)textureID;
			s_UIData.BufferPtr++;
		}
		s_UIData.IndexCount += 6 * text.size();
	}
	
	void InGuiRenderer::Flush()
	{
		uint32_t dataSize = (uint8_t*)s_UIData.BufferPtr - (uint8_t*)s_UIData.BufferBase;
		if (dataSize)
		{
			s_UIData.Material->Set("u_ViewportSize", s_UIData.Data.ViewportSize);

			s_UIData.Material->Bind();
			
			for (auto& pair : s_UIData.Textures)
			{
				Texture2D::Bind(pair.RendererID, pair.Slot);
			}
			
			s_UIData.QuadVertexBuffer->Update(s_UIData.BufferBase, dataSize);
			s_UIData.QuadVertexArray->Bind();
			RenderCommand::DrawIndexed(s_UIData.QuadVertexArray, s_UIData.IndexCount);
			s_UIData.Reset();
		}
	}
	void InGuiRenderer::EndScene()
	{
	}

	void RendererUIData::Reset()
	{
		if (!BufferBase)
		{
			BufferBase = new Vertex2D[MaxVertices];
			QuadVertexArray = VertexArray::Create();
			QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(Vertex2D));
			QuadVertexBuffer->SetLayout(BufferLayout{
			{0, XYZ::ShaderDataComponent::Float4, "a_Color" },
			{1, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{2, XYZ::ShaderDataComponent::Float2, "a_TexCoord" },
			{3, XYZ::ShaderDataComponent::Float,  "a_TextureID" },
				});
			QuadVertexArray->AddVertexBuffer(QuadVertexBuffer);


			uint32_t* quadIndices = new uint32_t[MaxIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, MaxIndices);
			QuadVertexArray->SetIndexBuffer(quadIB);
			delete[] quadIndices;
		}
		BufferPtr = BufferBase;
		IndexCount = 0;
		Textures.clear();
	}
}