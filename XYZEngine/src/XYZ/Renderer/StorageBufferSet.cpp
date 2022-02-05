#include "stdafx.h"
#include "StorageBufferSet.h"

#include "XYZ/API/Vulkan/VulkanStorageBufferSet.h"

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {
	Ref<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::Vulkan: return Ref<VulkanStorageBufferSet>::Create(frames);
		default:
			break;
		}
		XYZ_ASSERT(false, "Selected API not supported");
		return nullptr;
	}
}