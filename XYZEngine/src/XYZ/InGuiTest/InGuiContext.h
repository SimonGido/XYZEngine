#pragma once
#include "InGuiMeshFactory.h"
#include "InGuiAllocator.h"
#include "InGuiRenderer.h"
#include "InGuiInput.h"


namespace XYZ {
	class IGContext
	{
	public:
		IGAllocator	 Allocator;
		IGRenderData RenderData;
		IGMesh		 Mesh;
		IGInput		 Input;
	};
}