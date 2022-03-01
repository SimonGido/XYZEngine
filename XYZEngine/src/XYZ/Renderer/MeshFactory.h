#pragma once
#include "Mesh.h"

namespace XYZ {

	

	class MeshFactory
	{
	public:
		static Ref<MeshSource> CreateQuad(const glm::vec2& size);
		static Ref<Mesh> CreateInstancedQuad(const glm::vec2& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);
		
		static Ref<Mesh> CreateBox(const glm::vec3& size);
		static Ref<Mesh> CreateInstancedBox(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);

		// Cube is just a box with 24 vertices, required for texturing
		static Ref<Mesh> CreateCube(const glm::vec3& size, const BufferLayout& layout);
		static Ref<Mesh> CreateInstancedCube(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);
	};
}