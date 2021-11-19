#include "stdafx.h"
#include "Fence.h"

#include "XYZ/API/OpenGL/OpenGLFence.h"
#include "XYZ/API/Vulkan/VulkanFence.h"

#include "Renderer.h"


namespace XYZ {
	Ref<Fence> Fence::Create(uint64_t timeOut)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: return Ref<OpenGLFence>::Create(timeOut);
		case RendererAPI::API::Vulkan: return Ref<VulkanFence>::Create(timeOut);
		default:
			break;
		}
		return nullptr;
	}
}