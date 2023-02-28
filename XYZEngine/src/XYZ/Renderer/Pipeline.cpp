#include "Pipeline.h"

#include "XYZ/API/Vulkan/VulkanPipeline.h"
#include "Renderer.h"

namespace XYZ {

    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
    {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::Vulkan:  return Ref<VulkanPipeline>::Create(spec);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
    }
}