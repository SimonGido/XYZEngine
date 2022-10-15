#include "stdafx.h"
#include "IndirectBufferSet.h"

#include "XYZ/API/Vulkan/VulkanIndirectBufferSet.h"

namespace XYZ {
	Ref<IndirectBufferSet> XYZ::IndirectBufferSet::Create(uint32_t frames)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::Vulkan: return Ref<VulkanIndirectBufferSet>::Create(frames);
		default:
			break;
		}
		XYZ_ASSERT(false, "Selected API not supported");
		return nullptr;
	}
}
