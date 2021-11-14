#include "stdafx.h"
#include "RenderCommandBuffer.h"

#include "XYZ/Renderer/RendererAPI.h"
#include "XYZ/API/Vulkan/VulkanRenderCommandBuffer.h"

namespace XYZ {
	Ref<RenderCommandBuffer> RenderCommandBuffer::Create(uint32_t count, const std::string& debugName)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "API is not supported") return nullptr;
		case RendererAPI::API::Vulkan:  return Ref<VulkanRenderCommandBuffer>::Create(count, debugName);
		}
		XYZ_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}