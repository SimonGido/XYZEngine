#include "stdafx.h"
#include "Material.h"

#include "Renderer.h"
#include "XYZ/API/Vulkan/VulkanMaterial.h"



namespace XYZ {
	
	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::Vulkan: return Ref<VulkanMaterial>::Create(shader);
		default:
			break;
		}
		XYZ_ASSERT(false, "Not supported API");
		return nullptr;
	}

	void Material::invalidateInstances()
	{
		for (auto& instance : m_MaterialInstances)
			instance->allocateStorage(GetShader()->GetBuffers());
	}

	

	

}