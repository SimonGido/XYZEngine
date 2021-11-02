#pragma once

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "XYZ/Renderer/Font.h"

namespace XYZ {
	struct Renderer2DStats
	{
		uint32_t DrawCalls = 0;
		uint32_t LineDrawCalls = 0;
		uint32_t CollisionDrawCalls = 0;
		uint32_t FilledCircleDrawCalls = 0;
	};

	template <typename VertexType>
	struct Renderer2DBuffer
	{
		Renderer2DBuffer() = default;
		~Renderer2DBuffer();

		void Init(uint32_t maxVertices, uint32_t* indices, uint32_t indexCount, const BufferLayout& layout);
		void Reset();

		Ref<VertexArray>  VertexArray;
		Ref<VertexBuffer> VertexBuffer;
		VertexType*		  BufferBase = nullptr;
		VertexType*		  BufferPtr  = nullptr;
		uint32_t		  IndexCount = 0;
	};

	class Renderer2D : public RefCount
	{
	public:
		Renderer2D();
		~Renderer2D();

		void BeginScene();

		uint32_t SetTexture(const Ref<Texture>& texture);
		void	 SetMaterial(const Ref<Material>& material);

		void SubmitCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color = glm::vec4(1.0f));
		void SubmitFilledCircle(const glm::vec3& pos, float radius, float thickness, const glm::vec4& color = glm::vec4(1.0f));
		void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor = 1.0f);
		void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		void SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		void SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);

		void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));
		void SubmitLineQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color = glm::vec4(1));


		void SubmitCollisionQuad(const glm::mat4& transform, uint32_t id);

		void SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
	
		void FlushAll();
		void Flush();
		void FlushLines();
		void FlushCollisions();
		void FlushFilledCircles();
		/**
		* Clean up after rendering
		*/
		void EndScene();
		const Renderer2DStats& GetStats();

	private:
		void resetQuads();
		void resetLines();
		void resetCollisions();

	private:
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
			int		  CollisionID;
		};

		struct CircleVertex
		{
			glm::vec3 WorldPosition;
			float	  Thickness;
			glm::vec2 LocalPosition;
			glm::vec4 Color;
		};


		static constexpr uint32_t sc_MaxTextures = 32;
		static constexpr uint32_t sc_MaxQuads = 10000;
		static constexpr uint32_t sc_MaxVertices = sc_MaxQuads * 4;
		static constexpr uint32_t sc_MaxIndices = sc_MaxQuads * 6;
		static constexpr uint32_t sc_MaxLines = 10000;
		static constexpr uint32_t sc_MaxLineVertices = sc_MaxLines * 2;
		static constexpr uint32_t sc_MaxLineIndices = sc_MaxLines * 2;
		static constexpr uint32_t sc_MaxCollisionVertices = sc_MaxQuads * 4;
		static constexpr uint32_t sc_MaxPoints = 10000;
		static constexpr glm::vec4 sc_QuadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};
	private:
		Ref<Material>  m_DefaultQuadMaterial;
		Ref<Material>  m_QuadMaterial;
		Ref<Shader>    m_LineShader;
		Ref<Shader>    m_CollisionShader;
		Ref<Shader>    m_PointShader;
		Ref<Shader>	   m_CircleShader;

		Ref<Texture2D> m_WhiteTexture;
		Ref<Texture>   m_TextureSlots[sc_MaxTextures];
		uint32_t	   m_TextureSlotIndex = 0;


		Renderer2DBuffer<Vertex2D>		  m_QuadBuffer;
		Renderer2DBuffer<LineVertex>	  m_LineBuffer;
		Renderer2DBuffer<CollisionVertex> m_CollisionBuffer;
		Renderer2DBuffer<CircleVertex>	  m_CircleBuffer;

		Renderer2DStats    m_Stats;	
	};

	template<typename VertexType>
	inline Renderer2DBuffer<VertexType>::~Renderer2DBuffer()
	{
		if (BufferBase)
		{
			delete[]BufferBase;
		}
	}

	template<typename VertexType>
	inline void Renderer2DBuffer<VertexType>::Init(uint32_t maxVertices, uint32_t* indices, uint32_t indexCount, const BufferLayout& layout)
	{
		this->BufferBase = new VertexType[maxVertices];
		this->VertexArray = VertexArray::Create();
		this->VertexBuffer = VertexBuffer::Create(maxVertices * sizeof(VertexType));
		this->VertexBuffer->SetLayout(layout);
		this->VertexArray->AddVertexBuffer(VertexBuffer);
		this->VertexArray->SetIndexBuffer(IndexBuffer::Create(indices, indexCount));
		Reset();
	}
	template<typename VertexType>
	inline void Renderer2DBuffer<VertexType>::Reset()
	{
		BufferPtr = BufferBase;
		IndexCount = 0;
	}
}