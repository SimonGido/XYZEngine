#include "stdafx.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "XYZ/API/OpenGL/OpenGLVertexArray.h"


namespace XYZ {
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   XYZ_ASSERT(false, "Unknown API!");
		case RendererAPI::API::OpenGL: return Ref<OpenGLVertexArray>::Create();
		}
		return nullptr;
	}

}