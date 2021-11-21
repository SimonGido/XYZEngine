#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Buffer.h"

#include <glm/glm.hpp>

namespace XYZ {


	class Mesh : public RefCount
	{
	public:
		Mesh();
		Mesh(const Ref<Material>& material);

		void SetMaterial(const Ref<Material>& material);
		void SetIndices(uint32_t* indices, uint32_t count);
		void SetVertexBufferData(uint32_t index, const void* vertices, uint32_t size, uint32_t offset = 0);
		void AddVertexBuffer(const BufferLayout& layout, const void* vertices, uint32_t size, BufferUsage usage);
		void ClearVertexBuffers();

		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		Ref<Material>			GetMaterial()		   { return m_Material; }
		uint32_t				GetIndexCount()  const { return m_IndexBuffer->GetCount(); }
	private:
		void updateVertexArray();

	private:
		Ref<Material>					m_Material;
		Ref<VertexArray>				m_VertexArray;
		Ref<IndexBuffer>				m_IndexBuffer;
		std::vector<Ref<VertexBuffer>>	m_VertexBuffers;
	};
}
