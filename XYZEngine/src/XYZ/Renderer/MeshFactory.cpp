#include "stdafx.h"
#include "MeshFactory.h"

namespace XYZ {


    Ref<Mesh> MeshFactory::CreateQuad(const glm::vec2& size, const BufferLayout& layout)
    {
	    const Vertex quad[4] = {
			Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)},
			Vertex{glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
			Vertex{glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
			Vertex{glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f)}
		};
		Ref<Mesh> result = Ref<Mesh>::Create();
		result->AddVertexBuffer(layout, quad, 4 * sizeof(Vertex), BufferUsage::Static);
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
		const Vertex vertices[8] = {
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Down Left
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Down Right 
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, {}},  // Front Up   Right
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, {}},	 // Front Up   Left
			
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Down Left
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Down Right
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, {}},	// Back  Up	  Right
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, {}}	// Back  Up	  Left
		};

		const Triangle triangles[12] = {
			Triangle{0, 1, 2},
			Triangle{2, 3, 0},
			
			Triangle{1, 5, 6}, 
			Triangle{6, 2, 1},
			
			Triangle{7, 6, 5},
			Triangle{5, 4, 7},
			
			Triangle{4, 0, 3},
			Triangle{3, 7, 4},
			
			Triangle{4, 5, 1},
			Triangle{1, 0, 4},
			
			Triangle{3, 2, 6},
			Triangle{6, 7, 3}
		};
		Ref<Mesh> result = Ref<Mesh>::Create();
		result->AddVertexBuffer(layout, vertices, 8 * sizeof(Vertex), BufferUsage::Static);
		result->SetIndices((uint32_t*)triangles, 36);
		return result;
	}
	Ref<Mesh> MeshFactory::CreateInstancedBox(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<Mesh> result = CreateBox(size, layout);
		result->AddVertexBuffer(instanceLayout, nullptr, count * instanceLayout.GetStride(), BufferUsage::Dynamic);
		return result;
	}
	Ref<Mesh> MeshFactory::CreateCube(const glm::vec3& size, const BufferLayout& layout)
	{
		const Vertex vertices[24] = {
			// Front face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 1.0f)},

			// Top face
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(0.0f, 1.0f)},

			// Bottom face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(0.0f, 1.0f)},

			// Left face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 1.0f)},

			// Right face 
			Vertex{{  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f }, glm::vec2(0.0f, 1.0f)},

			// Back face
			Vertex{{ -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(0.0f, 0.0f)},
			Vertex{{  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 0.0f)},
			Vertex{{  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(1.0f, 1.0f)},
			Vertex{{ -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f }, glm::vec2(0.0f, 1.0f)}
		};

		

		Triangle triangles[12];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < 12; i += 2)
		{
			triangles[i]	 = Triangle{ offset, offset + 1, offset + 2 };
			triangles[i + 1] = Triangle{ offset + 2, offset + 3, offset };
			offset += 4;
		}
		Ref<Mesh> result = Ref<Mesh>::Create();
		result->AddVertexBuffer(layout, vertices, 24 * sizeof(Vertex), BufferUsage::Static);
		result->SetIndices((uint32_t*)triangles, 36);
		return result;
	}
	Ref<Mesh> MeshFactory::CreateInstancedCube(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count)
	{
		Ref<Mesh> result = CreateCube(size, layout);
		result->AddVertexBuffer(instanceLayout, nullptr, count * instanceLayout.GetStride(), BufferUsage::Dynamic);
		return result;
	}
}
