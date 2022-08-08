#include "stdafx.h"
#include "PipelineCompute.h"

#include "Renderer.h"
#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"

namespace XYZ {


	Ref<PipelineCompute> PipelineCompute::Create(Ref<Shader> computeShader)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None: return nullptr;
		case RendererAPI::Type::Vulkan: return Ref<VulkanPipelineCompute>::Create(computeShader);
		}
		XYZ_ASSERT(false, "Not supported API");
		return nullptr;
	}

}