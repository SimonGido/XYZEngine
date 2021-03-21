#pragma once
#include "IGMeshFactory.h"
#include "IGAllocator.h"
#include "IGRenderer.h"
#include "IGInput.h"
#include "IGDockspace.h"


namespace XYZ {

	struct IGFrameData
	{
		glm::vec2 MouseOffset;
	};

	class IGContext
	{
	public:
		IGAllocator	 Allocator;
		IGRenderData RenderData;
		IGMesh		 Mesh;
		IGInput		 Input;
		IGFrameData  FrameData;
		IGDockspace  Dockspace;
	};
}