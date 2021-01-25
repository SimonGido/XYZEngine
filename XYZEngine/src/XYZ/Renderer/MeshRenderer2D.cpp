#include "stdafx.h"
#include "MeshRenderer2D.h"

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
	};



	struct MeshRenderer2DData
	{
		void Reset();

		Ref<Material> Material;
		Ref<Shader> LineShader;

		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;


		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		uint32_t IndexCount = 0;
		uint32_t* IndexBufferBase = nullptr;
		uint32_t* IndexBufferPtr = nullptr;

		Vertex2D* BufferBase = nullptr;
		Vertex2D* BufferPtr = nullptr;


		SceneMeshRenderData Data;
		Renderer2DStats Stats;
	};


	void MeshRenderer2DData::Reset()
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

			IndexBufferBase = new uint32_t[MaxIndices];
		}
		IndexBufferPtr = IndexBufferBase;
		BufferPtr = BufferBase;
		IndexCount = 0;
	}

	static MeshRenderer2DData s_Data;

	void MeshRenderer2D::Init()
	{
		s_Data.Reset();
	}

	void MeshRenderer2D::Shutdown()
	{
		delete[] s_Data.BufferBase;
		delete[] s_Data.IndexBufferBase;
	}

	void MeshRenderer2D::BeginScene(const SceneMeshRenderData& data)
	{
		s_Data.Data = data;
	}

	void MeshRenderer2D::SetMaterial(const Ref<Material>& material)
	{
		if (s_Data.Material.Raw() && material.Raw() != s_Data.Material.Raw())
			Flush();

		s_Data.Material = material;
	}



	//void MeshRenderer2D::SubmitMesh(const glm::mat4& transform, const Ref<Mesh>& mesh)
	//{
	//	if (s_Data.IndexCount + mesh->Indices.size() >= s_Data.MaxIndices)
	//		Flush();
	//
	//	for (auto& vertex : mesh->Vertices)
	//	{
	//		s_Data.BufferPtr->Position = transform * vertex.Position;
	//		s_Data.BufferPtr->Color = vertex.Color;
	//		s_Data.BufferPtr->TexCoord = vertex.TexCoord;
	//		s_Data.BufferPtr->TextureID = (float)mesh->TextureID;
	//		s_Data.BufferPtr++;
	//	}
	//	for (auto index : mesh->Indices)
	//	{
	//		*s_Data.IndexBufferPtr = index;
	//		s_Data.IndexBufferPtr++;
	//	}
	//	s_Data.IndexCount += mesh->Indices.size();
	//}


	void MeshRenderer2D::Flush()
	{
		uint32_t dataSize = (uint8_t*)s_Data.BufferPtr - (uint8_t*)s_Data.BufferBase;
		if (dataSize)
		{
			s_Data.Material->Set("u_ViewProjectionMatrix", s_Data.Data.ViewProjectionMatrix);
			s_Data.Material->Bind();
			s_Data.QuadVertexBuffer->Update(s_Data.BufferBase, dataSize);
			s_Data.QuadVertexArray->Bind();

			Ref<IndexBuffer> quadIB = IndexBuffer::Create(s_Data.IndexBufferBase, s_Data.IndexCount);
			s_Data.QuadVertexArray->SetIndexBuffer(quadIB);

			Renderer::DrawIndexed(PrimitiveType::Triangles, s_Data.IndexCount);
			s_Data.Stats.DrawCalls++;
			s_Data.Reset();
		}
	}


	void MeshRenderer2D::EndScene()
	{
		s_Data.Stats.DrawCalls = 0;
	}
}