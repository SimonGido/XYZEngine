#include "stdafx.h"
#include "Material.h"

#include "Renderer.h"
#include "XYZ/API/Vulkan/VulkanMaterial.h"



namespace XYZ {



	Ref<MaterialInstance> Material::CreateMaterialInstance()
	{
		MaterialInstance* instance = new MaterialInstance(this);
		Ref<MaterialInstance> refInstance = Ref<MaterialInstance>(instance);
		m_MaterialInstances.push_back(refInstance);
		return refInstance;
	}
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
		for (auto it = m_MaterialInstances.begin(); it != m_MaterialInstances.end();)
		{
			if (it->IsValid())
			{
				(*it)->initializeStorage(GetShader()->GetBuffers());
				it++;
			}
			else
			{
				it = m_MaterialInstances.erase(it);
			}
		}
	}
}