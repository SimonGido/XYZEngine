#include "stdafx.h"

#include "FrameBuffer.h"
#include "API/OpenGL/OpenGLFrameBuffer.h"
#include "Renderer.h"


namespace XYZ {
	
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecs& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLFrameBuffer>::Create(specs);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}