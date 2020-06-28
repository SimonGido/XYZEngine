#include "stdafx.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "RenderCommand.h"


#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {	
	struct Vertex2D
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
		float	  TextureID;
	};

	struct Renderer2DData
	{
		void Reset();

		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;

		std::shared_ptr<VertexArray> QuadVertexArray;
		std::shared_ptr<VertexBuffer> QuadVertexBuffer;

		uint32_t IndexCount = 0;
		Vertex2D* BufferBase = nullptr;
		Vertex2D* BufferPtr = nullptr;


		glm::vec4 QuadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

	};
	void Renderer2DData::Reset()
	{
		if (!BufferBase)
		{
			BufferBase = new Vertex2D[MaxVertices];
			QuadVertexArray = VertexArray::Create();
			QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(Vertex2D));
			QuadVertexBuffer->SetLayout(BufferLayout{
			{0, XYZ::ShaderDataType::Float4, "a_Color" },
			{1, XYZ::ShaderDataType::Float3, "a_Position" },
			{2, XYZ::ShaderDataType::Float2, "a_TexCoord" },
			{3, XYZ::ShaderDataType::Float,  "a_TextureID" },
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
			std::shared_ptr<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, MaxIndices);
			QuadVertexArray->SetIndexBuffer(quadIB);
			delete[] quadIndices;
		}
		BufferPtr = BufferBase;
		IndexCount = 0;
	}


	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		s_Data.Reset();
	}

	void Renderer2D::Shutdown()
	{
	
	}

	void Renderer2D::BeginScene(const OrthoCamera& camera)
	{
		
	}

	void Renderer2D::Submit(CommandI& command, unsigned int size)
	{
		
	}


	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.IndexCount >= s_Data.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.BufferPtr->Color = color;
			s_Data.BufferPtr->TexCoord = texCoords[i];
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.IndexCount >= s_Data.MaxIndices)
			Flush();
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.BufferPtr->Color = color;
			s_Data.BufferPtr->TexCoord = glm::vec2(0);
			s_Data.BufferPtr->TextureID = 0.0f;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;

		if (s_Data.IndexCount >= s_Data.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.BufferPtr->Color = color;
			s_Data.BufferPtr->TexCoord = texCoords[i];
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;

		if (s_Data.IndexCount >= s_Data.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};

		glm::vec4 QuadVertexPositions[4] = {
			{ position.x - (size.x / 2), position.y - (size.y / 2), 0.0f, 1.0f },
			{ position.x + (size.x / 2), position.y - (size.y / 2), 0.0f, 1.0f },
			{ position.x + (size.x / 2), position.y + (size.y / 2), 0.0f, 1.0f },
			{ position.x - (size.x / 2), position.y + (size.y / 2), 0.0f, 1.0f }
		};

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = transform * QuadVertexPositions[i];
			s_Data.BufferPtr->Color = color;
			s_Data.BufferPtr->TexCoord = texCoords[i];
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const Quad& quad)
	{
		constexpr size_t quadVertexCount = 4;
		if (s_Data.IndexCount >= s_Data.MaxIndices)
			Flush();

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = transform * quad.Vertices[i].Position;
			s_Data.BufferPtr->Color = quad.Vertices[i].Color;
			s_Data.BufferPtr->TexCoord = quad.Vertices[i].TexCoord;
			s_Data.BufferPtr->TextureID = (float)quad.Vertices[i].TextureID;
			s_Data.BufferPtr++;
		}

		s_Data.IndexCount += 6;
	}

	void Renderer2D::Flush()
	{
		uint32_t dataSize = (uint8_t*)s_Data.BufferPtr - (uint8_t*)s_Data.BufferBase;
		s_Data.QuadVertexBuffer->Update(s_Data.BufferBase, dataSize);
		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.IndexCount);
		s_Data.Reset();
	}

	void Renderer2D::EndScene()
	{
	}

}