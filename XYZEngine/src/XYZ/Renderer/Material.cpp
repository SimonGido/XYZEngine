#include "stdafx.h"
#include "Material.h"

#include "Renderer.h"
#include "XYZ/API/Vulkan/VulkanMaterial.h"

namespace XYZ {
	
	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Ref<VulkanMaterial>::Create(shader);
		default:
			break;
		}
		XYZ_ASSERT(false, "Not supported API");
		return nullptr;
	}

}