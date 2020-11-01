#include "stdafx.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Renderer.h"


#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {	
	struct Renderer2DStats
	{
		uint32_t DrawCalls = 0;
		uint32_t LineDrawCalls = 0;
	};

	struct Vertex2D
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
		float	  TextureID;
		float	  TilingFactor;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer2DData
	{
		void Reset();
		void ResetLines();
		
		Ref<Material> QuadMaterial;
		Ref<Material> GridMaterial;
		Ref<Shader> LineShader;

		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;


		const uint32_t MaxLines = 10000;
		const uint32_t MaxLineVertices = MaxLines * 2;
		const uint32_t MaxLineIndices = MaxLines * 6;

		Ref<VertexArray> GridVertexArray;
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;


		uint32_t IndexCount = 0;
		Vertex2D* BufferBase = nullptr;
		Vertex2D* BufferPtr = nullptr;


		glm::vec4 QuadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;

		uint32_t LineIndexCount = 0;
		LineVertex* LineBufferBase = nullptr;
		LineVertex* LineBufferPtr = nullptr;

		glm::mat4 ViewProjectionMatrix;
		Renderer2DStats Stats;
	};


	void Renderer2DData::Reset()
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

			// Grid setup
			GridMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/Grid.glsl"));
			GridMaterial->Set("u_Scale", glm::vec2{ 16.025f,16.025f });
			GridMaterial->Set("u_LineWidth", 0.025f);
			struct QuadVertex
			{
				glm::vec3 Position;
				glm::vec2 TexCoord;
			};
			float x = -1;
			float y = -1;
			float width = 2, height = 2;

			QuadVertex data[4];
			data[0].Position = glm::vec3(x, y, 0.0f);
			data[0].TexCoord = glm::vec2(0, 0);
			
			data[1].Position = glm::vec3(x + width, y, 0.0f);
			data[1].TexCoord = glm::vec2(1, 0);

			data[2].Position = glm::vec3(x + width, y + height, 0.0f);
			data[2].TexCoord = glm::vec2(1, 1);

			data[3].Position = glm::vec3(x, y + height, 0.0f);
			data[3].TexCoord = glm::vec2(0, 1);

			GridVertexArray = VertexArray::Create();
			auto gridVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
			gridVB->SetLayout({
				{0, ShaderDataComponent::Float3, "a_Position" },
				{1, ShaderDataComponent::Float2, "a_TexCoord" }
			});
			GridVertexArray->AddVertexBuffer(gridVB);

			uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
			auto gridIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
	
			GridVertexArray->SetIndexBuffer(gridIB);
		}
		BufferPtr = BufferBase;	
		IndexCount = 0;	
	}
	void Renderer2DData::ResetLines()
	{
		if (!LineBufferBase)
		{	// Lines
			LineVertexArray = VertexArray::Create();

			LineShader = Shader::Create("Assets/Shaders/LineShader.glsl");
			LineVertexBuffer = VertexBuffer::Create(MaxLineVertices * sizeof(LineVertex));
			LineVertexBuffer->SetLayout({
				{0, XYZ::ShaderDataComponent::Float3, "a_Position" },
				{1, XYZ::ShaderDataComponent::Float4, "a_Color" },
				});
			LineBufferBase = new LineVertex[MaxLineVertices];

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

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		s_Data.Reset();
		s_Data.ResetLines();
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Data.BufferBase;
		delete[] s_Data.LineBufferBase;
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProjectionMatrix)
	{
		s_Data.ViewProjectionMatrix = viewProjectionMatrix;
	}

	void Renderer2D::SetMaterial(const Ref<Material>& material)
	{
		if (s_Data.QuadMaterial && material.Raw() != s_Data.QuadMaterial.Raw())
			Flush();

		s_Data.QuadMaterial = material;
		s_Data.QuadMaterial->Bind();
	}


	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor)
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
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
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
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
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
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (s_Data.LineIndexCount >= s_Data.MaxLineIndices)
			FlushLines();

		s_Data.LineBufferPtr->Position = p0;
		s_Data.LineBufferPtr->Color = color;
		s_Data.LineBufferPtr++;

		s_Data.LineBufferPtr->Position = p1;
		s_Data.LineBufferPtr->Color = color;
		s_Data.LineBufferPtr++;

		s_Data.LineIndexCount += 2;
	}

	void Renderer2D::SubmitParticles(const glm::mat4& transform, const Ref<ParticleEffect>& particleEffect)
	{
		particleEffect->GetVertexArray()->Bind();
		particleEffect->GetShaderStorage()->BindBase(1);
		particleEffect->GetIndirectBuffer()->Bind();
		Renderer::DrawElementsIndirect(nullptr);
	}


	void Renderer2D::SubmitGrid(const glm::mat4& transform, const glm::vec2& scale, float lineWidth)
	{
		auto shader = s_Data.GridMaterial->GetShader();
		s_Data.GridMaterial->Bind();
		shader->SetMat4("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);
		shader->SetFloat2("u_Scale", scale);
		shader->SetFloat(("u_LineWidth"), lineWidth);

		s_Data.GridVertexArray->Bind();
		Renderer::DrawIndexed(PrimitiveType::Triangles, 6);
	}

	void Renderer2D::Flush()
	{	
		uint32_t dataSize = (uint8_t*)s_Data.BufferPtr - (uint8_t*)s_Data.BufferBase;
		if (dataSize)
		{
			s_Data.QuadMaterial->Set("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);
			s_Data.QuadMaterial->Bind();
			s_Data.QuadVertexBuffer->Update(s_Data.BufferBase, dataSize);
			s_Data.QuadVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Triangles, s_Data.IndexCount);
			s_Data.Stats.DrawCalls++;
			s_Data.Reset();
		}	
	}
	void Renderer2D::FlushLines()
	{	
		uint32_t dataSize = (uint8_t*)s_Data.LineBufferPtr - (uint8_t*)s_Data.LineBufferBase;
		if (dataSize)
		{
			s_Data.LineShader->Bind();
			s_Data.LineShader->SetMat4("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);

			s_Data.LineVertexBuffer->Update(s_Data.LineBufferBase, dataSize);
			s_Data.LineVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Lines, s_Data.LineIndexCount);

			s_Data.Stats.LineDrawCalls++;
			s_Data.ResetLines();
		}	
	}

	void Renderer2D::EndScene()
	{
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.LineDrawCalls = 0;
		s_Data.QuadMaterial = nullptr;
	}
}