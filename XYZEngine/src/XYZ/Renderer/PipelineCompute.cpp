#include "stdafx.h"
#include "PipelineCompute.h"

#include "Renderer.h"
#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"

namespace XYZ {


	Ref<PipelineCompute> PipelineCompute::Create(const PipelineComputeSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None: return nullptr;
		case RendererAPI::Type::Vulkan: return Ref<VulkanPipelineCompute>::Create(specification);
		}
		XYZ_ASSERT(false, "Not supported API");
		return nullptr;
	}

}