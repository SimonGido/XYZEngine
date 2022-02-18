#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		uint32_t IDs[4] = { 0,0,0,0 };
		float Weights[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	};

	class Mesh : public Asset
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		virtual ~Mesh() override;

		const std::vector<Vertex>&   GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		Ref<VertexBuffer>   GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_IndexBuffer; }

		static AssetType	GetStaticType() { return AssetType::None; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;

		std::vector<Vertex>   m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
	
	class AnimatedMesh : public Asset
	{
	public:
		AnimatedMesh(std::vector<AnimatedVertex> vertices, std::vector<uint32_t> indices);
		
		const std::vector<AnimatedVertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>&	   GetIndices() const { return m_Indices; }

		Ref<VertexBuffer>   GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_IndexBuffer; }

		static AssetType	GetStaticType() { return AssetType::None; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;

		std::vector<AnimatedVertex> m_Vertices;
		std::vector<uint32_t>		m_Indices;
		
	};
}
