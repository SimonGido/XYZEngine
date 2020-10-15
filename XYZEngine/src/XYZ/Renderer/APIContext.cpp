#include "stdafx.h"
#include "APIContext.h"

#include "Renderer.h"
#include "RendererAPI.h"
#include "API/OpenGL/OpenGLAPIContext.h"


namespace XYZ {
	Ref<APIContext> APIContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLAPIContext>::Create(static_cast<GLFWwindow*>(window));
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}