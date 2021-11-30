#include "stdafx.h"
#include "RenderCommandBuffer.h"

#include "XYZ/Renderer/RendererAPI.h"
#include "XYZ/API/OpenGL/OpenGLRenderCommandBuffer.h"
#include "XYZ/API/Vulkan/VulkanRenderCommandBuffer.h"

namespace XYZ {
	Ref<RenderCommandBuffer> RenderCommandBuffer::Create(uint32_t count, const std::string& debugName)
	{
		switch (RendererAPI::GetType())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "API is not supported") return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLRenderCommandBuffer>::Create(count, debugName);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanRenderCommandBuffer>::Create(count, debugName);
		}
		XYZ_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}