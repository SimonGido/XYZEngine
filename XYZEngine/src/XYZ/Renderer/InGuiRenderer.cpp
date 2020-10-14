#include "stdafx.h"
#include "InGuiRenderer.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "XYZ/Renderer/Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace XYZ {

	struct RendererUIData
	{
		void Reset();
		void ResetLines();

		Ref<Material> Material;
		Ref<Shader> LineShader;

		static const uint32_t MaxTextures = 32;
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;


		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVertices = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 6;


		std::vector<TextureRendererIDPair> Textures;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		uint32_t IndexCount = 0;
		InGuiVertex* BufferBase = nullptr;
		InGuiVertex* BufferPtr = nullptr;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;

		uint32_t LineIndexCount = 0;
		InGuiLineVertex* LineBufferBase = nullptr;
		InGuiLineVertex* LineBufferPtr = nullptr;


		UIRenderData Data;
	};
	static RendererUIData s_UIData;

	void InGuiRenderer::Init()
	{
		s_UIData.Reset();
		s_UIData.ResetLines();
	}
	void InGuiRenderer::Shutdown()
	{
		delete[] s_UIData.BufferBase;
		delete[]s_UIData.LineBufferBase;
	}
	void InGuiRenderer::BeginScene(const UIRenderData& data)
	{
		s_UIData.Data = data;
	}
	void InGuiRenderer::SetTexturePairs(const std::vector<TextureRendererIDPair>& texturePairs)
	{
		for (auto pair : texturePairs)
		{
			s_UIData.Textures.push_back({ pair.TextureID, pair.RendererID });
		}
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
				textureID = s_UIData.Textures[i].TextureID;
				break;
			}
		}
		if (!textureID)
		{
			textureID += s_UIData.Material->GetNumberOfTextures();
			s_UIData.Textures.push_back({ textureID, rendererID });
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
		if (s_UIData.IndexCount + indexCount > s_UIData.MaxIndices)
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

	void InGuiRenderer::SubmitUI(const InGuiMesh& mesh)
	{
		uint32_t indexCount = (mesh.Vertices.size() / 4) * 6;
		if (s_UIData.IndexCount + indexCount >= s_UIData.MaxIndices ||
			s_UIData.Textures.size() + s_UIData.Material->GetNumberOfTextures()
			>= s_UIData.MaxTextures)
			Flush();

	
		for (auto & vertex : mesh.Vertices)
		{
			s_UIData.BufferPtr->Position = vertex.Position;
			s_UIData.BufferPtr->Color = vertex.Color;
			s_UIData.BufferPtr->TexCoord = vertex.TexCoord;
			s_UIData.BufferPtr->TextureID = vertex.TextureID;
			s_UIData.BufferPtr->TilingFactor = vertex.TilingFactor;
			s_UIData.BufferPtr++;
		}
		s_UIData.IndexCount += indexCount;
	}

	void InGuiRenderer::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (s_UIData.LineIndexCount >= RendererUIData::MaxLineIndices)
			FlushLines();

		s_UIData.LineBufferPtr->Position = p0;
		s_UIData.LineBufferPtr->Color = color;
		s_UIData.LineBufferPtr++;

		s_UIData.LineBufferPtr->Position = p1;
		s_UIData.LineBufferPtr->Color = color;
		s_UIData.LineBufferPtr++;

		s_UIData.LineIndexCount += 2;
	}

	void InGuiRenderer::SubmitLineMesh(const InGuiLineMesh& mesh)
	{
		if (mesh.Vertices.size() + s_UIData.LineIndexCount > RendererUIData::MaxLineIndices)
			FlushLines();

		for (auto& vertex : mesh.Vertices)
		{
			s_UIData.LineBufferPtr->Position = vertex.Position;
			s_UIData.LineBufferPtr->Color = vertex.Color;
			s_UIData.LineBufferPtr++;

			s_UIData.LineIndexCount++;
		}
	}


	void InGuiRenderer::Flush()
	{
		uint32_t dataSize = (uint8_t*)s_UIData.BufferPtr - (uint8_t*)s_UIData.BufferBase;
		if (dataSize)
		{
			s_UIData.Material->Set("u_ViewportSize", s_UIData.Data.ViewportSize);
			s_UIData.Material->Set("u_ViewProjection", s_UIData.Data.ViewProjection);

			s_UIData.Material->Bind();

			for (auto& pair : s_UIData.Textures)
			{
				Texture2D::Bind(pair.RendererID, pair.TextureID);
			}

			s_UIData.QuadVertexBuffer->Update(s_UIData.BufferBase, dataSize);
			s_UIData.QuadVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Triangles, s_UIData.IndexCount);		
		
			s_UIData.Reset();
		}
	}
	void InGuiRenderer::FlushLines()
	{	
		uint32_t dataSize = (uint8_t*)s_UIData.LineBufferPtr - (uint8_t*)s_UIData.LineBufferBase;
		if (dataSize)
		{
			s_UIData.LineShader->Bind();
			s_UIData.LineShader->SetFloat2("u_ViewportSize", s_UIData.Data.ViewportSize);
			s_UIData.LineShader->SetMat4("u_ViewProjection", s_UIData.Data.ViewProjection);

			s_UIData.LineVertexBuffer->Update(s_UIData.LineBufferBase, dataSize);
			s_UIData.LineVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Lines, s_UIData.LineIndexCount);

			s_UIData.ResetLines();
		}
	}
	void InGuiRenderer::EndScene()
	{
		
	}

	void RendererUIData::Reset()
	{
		if (!BufferBase)
		{
			BufferBase = new InGuiVertex[MaxVertices];
			QuadVertexArray = VertexArray::Create();
			QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(InGuiVertex));
			QuadVertexBuffer->SetLayout(BufferLayout{
			{0, XYZ::ShaderDataComponent::Float4, "a_Color" },
			{1, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{2, XYZ::ShaderDataComponent::Float2, "a_TexCoord" },
			{3, XYZ::ShaderDataComponent::Float,  "a_TextureID" },
			{4, XYZ::ShaderDataComponent::Float,  "a_TilingFactor" },
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
	void RendererUIData::ResetLines()
	{
		if (!LineBufferBase)
		{
			LineVertexArray = VertexArray::Create();
			LineShader = Shader::Create("Assets/Shaders/LineInGuiShader.glsl");
			LineVertexBuffer = VertexBuffer::Create(MaxLineVertices * sizeof(InGuiLineVertex));
			LineVertexBuffer->SetLayout({
				{0, XYZ::ShaderDataComponent::Float3, "a_Position" },
				{1, XYZ::ShaderDataComponent::Float4, "a_Color" },
				});
			LineBufferBase = new InGuiLineVertex[MaxLineVertices];

			LineVertexArray->AddVertexBuffer(LineVertexBuffer);
			uint32_t* lineIndices = new uint32_t[MaxLineIndices];
			for (uint32_t i = 0; i < MaxLineIndices; i++)
				lineIndices[i] = i;

			Ref<IndexBuffer> lineIndexBuffer = IndexBuffer::Create(lineIndices, MaxLineIndices);
			LineVertexArray->SetIndexBuffer(lineIndexBuffer);
			delete[] lineIndices;
		}
		LineBufferPtr = LineBufferBase;
		LineIndexCount = 0;
	}

}