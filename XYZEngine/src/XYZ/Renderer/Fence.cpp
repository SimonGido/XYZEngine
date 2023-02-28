#include "stdafx.h"
#include "Fence.h"


#include "XYZ/API/Vulkan/VulkanFence.h"

#include "Renderer.h"


namespace XYZ {
	Ref<Fence> Fence::Create(uint64_t timeOut)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:   XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");

		case RendererAPI::Type::Vulkan: return Ref<VulkanFence>::Create(timeOut);
		default:
			break;
		}
		return nullptr;
	}
}