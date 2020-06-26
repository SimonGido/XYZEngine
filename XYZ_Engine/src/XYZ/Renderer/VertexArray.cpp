#include "stdafx.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "API/OpenGL/OpenGLVertexArray.h"


namespace XYZ {
	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   XYZ_ASSERT(false, "Unknown API!");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
		}
		return std::shared_ptr<VertexArray>();
	}

}