#include "stdafx.h"

#include "Framebuffer.h"
#include "XYZ/API/Vulkan/VulkanFramebuffer.h"

#include "Renderer.h"


namespace XYZ {
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::Vulkan:  return Ref<VulkanFramebuffer>::Create(specs);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}