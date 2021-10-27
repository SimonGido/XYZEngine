#pragma once
#include "Mesh.h"

namespace XYZ {
	class MeshFactory
	{
	public:
		static Ref<Mesh> CreateQuad(const glm::vec2& size,const BufferLayout& layout);
		static Ref<Mesh> CreateInstancedQuad(const glm::vec2& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);
		
		static Ref<Mesh> CreateBox(const glm::vec3& size, const BufferLayout& layout);
		static Ref<Mesh> CreateInstancedBox(const glm::vec3& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);

	};
}