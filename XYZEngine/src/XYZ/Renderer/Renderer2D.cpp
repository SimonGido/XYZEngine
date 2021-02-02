#include "stdafx.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Renderer.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {	
	struct Renderer2DStats
	{
		uint32_t DrawCalls = 0;
		uint32_t LineDrawCalls = 0;
		uint32_t CollisionDrawCalls = 0;
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
	
	struct CollisionVertex
	{
		glm::vec3 Position;
		int CollisionID;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxTextures = 32;
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVertices = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 6;
		static const uint32_t MaxCollisionVertices = MaxQuads * 4;


		void Reset();
		void ResetLines();
		void ResetCollisions();
		
		Ref<Material> DefaultQuadMaterial;
		Ref<Material> QuadMaterial;
		Ref<Material> GridMaterial;
		Ref<Shader> LineShader;
		Ref<Shader> CollisionShader;

		Ref<Texture> TextureSlots[MaxTextures];
		uint32_t TextureSlotIndex = 0;
	
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


		Ref<VertexArray>  CollisionVertexArray;
		Ref<VertexBuffer> CollisionVertexBuffer;

		uint32_t CollisionIndexCount = 0;
		CollisionVertex* CollisionBufferBase = nullptr;
		CollisionVertex* CollisionBufferPtr = nullptr;

		glm::mat4 ViewProjectionMatrix;
		Renderer2DStats Stats;
	};


	void Renderer2DData::Reset()
	{
		if (!BufferBase)
		{
			DefaultQuadMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
			BufferBase = new Vertex2D[MaxVertices];
			QuadVertexArray = VertexArray::Create();
			QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(Vertex2D));
			QuadVertexBuffer->SetLayout(BufferLayout{
			{0, XYZ::ShaderDataComponent::Float4, "a_Color" },
			{1, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{2, XYZ::ShaderDataComponent::Float2, "a_TexCoord" },
			{3, XYZ::ShaderDataComponent::Float,  "a_TextureID" },
			{4, XYZ::ShaderDataComponent::Float,  "a_TilingFactor" }
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
		QuadMaterial = DefaultQuadMaterial;
		TextureSlotIndex = 0;
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

	void Renderer2DData::ResetCollisions()
	{
		if (!CollisionBufferBase)
		{	// Lines
			CollisionVertexArray = VertexArray::Create();

			CollisionShader = Shader::Create("Assets/Shaders/MousePicker.glsl");
			CollisionVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(CollisionVertex));
			CollisionVertexBuffer->SetLayout({
				{0, XYZ::ShaderDataComponent::Float3, "a_Position" },
				{1, XYZ::ShaderDataComponent::Int,    "a_ObjectID" },
				});
			CollisionBufferBase = new CollisionVertex[MaxCollisionVertices];

			CollisionVertexArray->AddVertexBuffer(CollisionVertexBuffer);
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
			Ref<IndexBuffer> collisionIndexBuffer = IndexBuffer::Create(quadIndices, MaxIndices);
			CollisionVertexArray->SetIndexBuffer(collisionIndexBuffer);
			delete[] quadIndices;
		}

		CollisionBufferPtr = CollisionBufferBase;
		CollisionIndexCount = 0;
	}
	
	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		s_Data.Reset();
		s_Data.ResetLines();
		s_Data.ResetCollisions();
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Data.BufferBase;
		delete[] s_Data.LineBufferBase;
		delete[] s_Data.CollisionBufferBase;
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProjectionMatrix)
	{
		s_Data.ViewProjectionMatrix = viewProjectionMatrix;
		s_Data.QuadMaterial = s_Data.DefaultQuadMaterial;
	}

	uint32_t Renderer2D::SetTexture(const Ref<Texture>& texture)
	{
		for (uint32_t i = 0; i < s_Data.QuadMaterial->GetTextures().size(); ++i)
		{
			if (s_Data.QuadMaterial->GetTextures()[i].Raw() && s_Data.QuadMaterial->GetTextures()[i]->GetRendererID() == texture->GetRendererID())
				return i;
		}
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
		{
			if (s_Data.TextureSlots[i]->GetRendererID() == texture->GetRendererID())
				return i + s_Data.QuadMaterial->GetTextures().size();
		}

		if (s_Data.QuadMaterial->GetTextures().size() + s_Data.TextureSlotIndex >= s_Data.MaxTextures + 1)
			Flush();
		
		s_Data.TextureSlots[s_Data.TextureSlotIndex++] = texture;
		return s_Data.TextureSlotIndex + s_Data.QuadMaterial->GetTextures().size() - 1;
	}

	void Renderer2D::SetMaterial(const Ref<Material>& material)
	{
		XYZ_ASSERT(material.Raw(), "Material can not be null");
		if (*material.Raw() != *s_Data.QuadMaterial.Raw())
			Flush();
		
		s_Data.QuadMaterial = material;
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

		if (s_Data.IndexCount + 6 >= s_Data.MaxIndices)
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

	void Renderer2D::SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (s_Data.IndexCount + 6 >= s_Data.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  position.x - size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y - size.y / 2.0f, 0.0f},
			{  position.x + size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f},
			{  position.x - size.x / 2.0f,  position.y + size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = vertices[i];
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

	void Renderer2D::SubmitCollisionQuad(const glm::mat4& transform, uint32_t id)
	{
		constexpr size_t quadVertexCount = 4;

		if (s_Data.CollisionIndexCount + 6 >= s_Data.MaxIndices)
			FlushCollisions();

		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.CollisionBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.CollisionBufferPtr->CollisionID = (int)id;
			s_Data.CollisionBufferPtr++;
		}
		s_Data.CollisionIndexCount += 6;
	}


	void Renderer2D::SubmitQuads(const Vertex* vertices, uint32_t countQuads, uint32_t textureID, float tilingFactor)
	{
		if (s_Data.IndexCount + countQuads * 6 >= s_Data.MaxIndices)
			Flush();

		for (uint32_t i = 0; i < countQuads * 4; ++i)
		{
			s_Data.BufferPtr->Position = vertices[i].Position;
			s_Data.BufferPtr->Color = vertices[i].Color;
			s_Data.BufferPtr->TexCoord = vertices[i].TexCoord;
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
	}

	void Renderer2D::SubmitQuads(const glm::mat4& transform, const Vertex* vertices, uint32_t countQuads, uint32_t textureID, float tilingFactor)
	{
		if (s_Data.IndexCount + countQuads * 6 >= s_Data.MaxIndices)
			Flush();

		for (uint32_t i = 0; i < countQuads * 4; ++i)
		{
			s_Data.BufferPtr->Position = transform * glm::vec4(vertices[i].Position, 1.0f);
			s_Data.BufferPtr->Color = vertices[i].Color;
			s_Data.BufferPtr->TexCoord = vertices[i].TexCoord;
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6 * countQuads;
	}

	void Renderer2D::SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;

		if (s_Data.IndexCount + 6 >= s_Data.MaxIndices)
			Flush();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};

		glm::vec3 vertices[quadVertexCount] = {
			{  position.x ,			 position.y, 0.0f},
			{  position.x + size.x,  position.y, 0.0f},
			{  position.x + size.x,  position.y + size.y, 0.0f},
			{  position.x,			 position.y + size.y, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.BufferPtr->Position = vertices[i];
			s_Data.BufferPtr->Color = color;
			s_Data.BufferPtr->TexCoord = texCoords[i];
			s_Data.BufferPtr->TextureID = (float)textureID;
			s_Data.BufferPtr->TilingFactor = tilingFactor;
			s_Data.BufferPtr++;
		}
		s_Data.IndexCount += 6;
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
			XYZ_ASSERT(s_Data.QuadMaterial.Raw(), "No material set");
				
			s_Data.QuadMaterial->Bind();
			uint32_t textureSlotOffset = s_Data.QuadMaterial->GetTextures().size();
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
				s_Data.TextureSlots[i]->Bind(i + textureSlotOffset);

			s_Data.QuadMaterial->GetShader()->SetMat4("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);
					
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

	void Renderer2D::FlushCollisions()
	{
		uint32_t dataSize = (uint8_t*)s_Data.CollisionBufferPtr - (uint8_t*)s_Data.CollisionBufferBase;
		if (dataSize)
		{
			s_Data.CollisionShader->Bind();
			s_Data.CollisionShader->SetMat4("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);

			s_Data.CollisionVertexBuffer->Update(s_Data.CollisionBufferBase, dataSize);
			s_Data.CollisionVertexArray->Bind();
			Renderer::DrawIndexed(PrimitiveType::Triangles, s_Data.CollisionIndexCount);

			s_Data.Stats.CollisionDrawCalls++;
			s_Data.ResetCollisions();
		}
	}



	void Renderer2D::EndScene()
	{
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.LineDrawCalls = 0;
		s_Data.Stats.CollisionDrawCalls = 0;
	}
}