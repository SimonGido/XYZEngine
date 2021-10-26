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
	}
}
