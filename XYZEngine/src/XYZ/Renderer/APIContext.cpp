#include "stdafx.h"
#include "APIContext.h"

#include "Renderer.h"
#include "RendererAPI.h"
#include "XYZ/API/OpenGL/OpenGLAPIContext.h"
#include "XYZ/API/Vulkan/VulkanContext.h"

namespace XYZ {
	Ref<APIContext> APIContext::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLAPIContext>::Create();
		case RendererAPI::API::Vulkan:  return Ref<VulkanContext>::Create();
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}