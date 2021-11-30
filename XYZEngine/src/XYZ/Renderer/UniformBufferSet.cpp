#include "stdafx.h"
#include "UniformBufferSet.h"

#include "XYZ/API/Vulkan/VulkanUniformBufferSet.h"
//#include "Renderer.h"

namespace XYZ {
	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t frames)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::Vulkan: return Ref<VulkanUniformBufferSet>::Create(frames);
		default:
			break;
		}
		XYZ_ASSERT(false, "Selected API not supported");
		return nullptr;
	}
}