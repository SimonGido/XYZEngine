#include "stdafx.h"
#include "RenderPass.h"

#include "RendererAPI.h"
#include "XYZ/API/OpenGL/OpenGLRenderPass.h"
#include "XYZ/API/Vulkan/VulkanRenderPass.h"

namespace XYZ {
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{		
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLRenderPass>::Create(spec);
		case RendererAPI::API::Vulkan:  return Ref<VulkanRenderPass>::Create(spec);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}