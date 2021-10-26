#pragma once
#include "Mesh.h"

namespace XYZ {
	class MeshFactory
	{
	public:
		Ref<Mesh> CreateQuad(const glm::vec2& size,const BufferLayout& layout);
		Ref<Mesh> CreateInstancedQuad(const glm::vec2& size, const BufferLayout& layout, const BufferLayout& instanceLayout, uint32_t count);
	};
}