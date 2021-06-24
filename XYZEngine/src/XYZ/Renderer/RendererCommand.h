#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"

namespace XYZ {

	class RendererCommand
	{
	public:
		virtual ~RendererCommand() = default;

		virtual void Bind() = 0;

		Ref<Material>    Material;
		uint32_t		 SortLayer;
	};

}