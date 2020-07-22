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
			{ position.x ,		  position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y + size.y, 0.0f, 1.0f },
			{ position.x ,        position.y + size.y, 0.0f, 1.0f }
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
			{ position.x ,		  position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y , 0.0f, 1.0f },
			{ position.x + size.x,position.y + size.y, 0.0f, 1.0f },
			{ position.x ,        position.y + size.y, 0.0f, 1.0f }
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

	void InGuiRenderer::SubmitUI(const glm::vec2& position, const Vertex* vertex, size_t count, uint32_t textureID)
	{
		uint32_t indexCount = (count / 4) * 6;
		if (s_UIData.IndexCount +  indexCount > s_UIData.MaxIndices)
			Flush();

		for (size_t i = 0; i < count; ++i)
		{
			s_UIData.BufferPtr->Position = vertex[i].Position + glm::vec4(position.x, position.y, 0, 0);
			s_UIData.BufferPtr->Color = vertex[i].Color;
			s_UIData.BufferPtr->TexCoord = vertex[i].TexCoord;
			s_UIData.BufferPtr->TextureID = (float)textureID;
			s_UIData.BufferPtr++;
		}
		s_UIData.IndexCount += indexCount;
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