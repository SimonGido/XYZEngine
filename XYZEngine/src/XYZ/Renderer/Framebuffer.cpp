#include "stdafx.h"

#include "Framebuffer.h"
#include "XYZ/API/OpenGL/OpenGLFramebuffer.h"
#include "Renderer.h"


namespace XYZ {
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecs& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLFramebuffer>::Create(specs);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}