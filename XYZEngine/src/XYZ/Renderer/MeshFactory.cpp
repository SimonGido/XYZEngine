#include "stdafx.h"
#include "MeshFactory.h"

namespace XYZ {
    Ref<Mesh> MeshFactory::CreateQuad(const glm::vec2& size, const BufferLayout& layout)
    {
		glm::vec3 quad[4] = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f,  0.5f, 0.0f),
			glm::vec3(-0.5f,  0.5f, 0.0f)
		};
		Ref<Mesh> result = Ref<Mesh>::Create();
		result->AddVertexBuffer(layout, quad, 4 * sizeof(glm::vec3), BufferUsage::Static);
		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };
		result->SetIndices(squareIndices, 6);
		return result;
    }
	Ref<Mesh> MeshFactory::CreateInstancedQuad(const glm::vec2& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<Mesh> result = CreateQuad(size, layout);
		result->AddVertexBuffer(instanceLayout, nullptr, count * instanceLayout.GetStride(), BufferUsage::Dynamic);
		return result;
		return result;
	}
	Ref<Mesh> MeshFactory::CreateBox(const glm::vec3& size, const BufferLayout& layout)
	{
		glm::vec3 positions[8] = {
			{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },
			{ size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f },
			{ size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },
			{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f },
			{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },
			{ size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f },
			{ size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f },
			{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }	
		};
		uint32_t indices[36] = {
			0, 1, 2,
			2, 3, 0,
			1, 5, 6,
			6, 2, 1,
			7, 6, 5,
			5, 4, 7,
			4, 0, 3,
			3, 7, 4,
			4, 5, 1,
			1, 0, 4,
			3, 2, 6,
			6, 7, 3
		};
		Ref<Mesh> result = Ref<Mesh>::Create();
		result->AddVertexBuffer(layout, positions, 8 * sizeof(glm::vec3), BufferUsage::Static);
		result->SetIndices(indices, 36);
		return result;
	}
	Ref<Mesh> MeshFactory::CreateInstancedBox(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<Mesh> result = CreateBox(size, layout);
		result->AddVertexBuffer(instanceLayout, nullptr, count * instanceLayout.GetStride(), BufferUsage::Dynamic);
		return result;
	}
}
