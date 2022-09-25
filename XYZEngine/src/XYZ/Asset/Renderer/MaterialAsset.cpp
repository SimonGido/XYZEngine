#include "stdafx.h"
#include "MaterialAsset.h"


namespace XYZ {

	MaterialAsset::MaterialAsset(Ref<ShaderAsset> shaderAsset)
		:
		m_ShaderAsset(shaderAsset)
	{
		m_Material = Material::Create(shaderAsset->GetShader());
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);

		setupTextureBuffers();
	}
	MaterialAsset::~MaterialAsset()
	{
	}
	void MaterialAsset::SetShaderAsset(Ref<ShaderAsset> shaderAsset)
	{
		XYZ_ASSERT(!IsFlagSet(AssetFlag::ReadOnly), "Asset is readonly");
		m_ShaderAsset = shaderAsset;
		m_Material = Material::Create(shaderAsset->GetShader());
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);
		setupTextureBuffers();
	}
	void MaterialAsset::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		XYZ_ASSERT(!IsFlagSet(AssetFlag::ReadOnly), "Asset is readonly");
		for (auto& it : m_Textures)
		{
			if (it.Name == name)
			{
				it.Texture = texture;
				m_Material->SetImage(it.Name, texture->GetImage());
				return;
			}
		}
		XYZ_ASSERT(false, "Texture does not exist");
	}
	void MaterialAsset::SetTexture(const std::string& name, Ref<Texture2D> texture, uint32_t index)
	{
		XYZ_ASSERT(!IsFlagSet(AssetFlag::ReadOnly), "Asset is readonly");
		const size_t arrayIndex = static_cast<size_t>(index);
		for (auto& texArray : m_TextureArrays)
		{
			if (texArray.Name == name)
			{
				texArray.Textures[index] = texture;
				m_Material->SetImageArray(texArray.Name, texture->GetImage(), index);
				return;
			}
		}
		XYZ_ASSERT(false, "Texture does not exist");
	}
	void MaterialAsset::setupTextureBuffers()
	{
		m_Textures.clear();
		m_TextureArrays.clear();
		auto& resources = m_ShaderAsset->GetShader()->GetResources();
		for (auto& [name, resourceDecl] : resources)
		{
			if (resourceDecl.GetType() == ShaderResourceType::Sampler2D)
			{
				if (resourceDecl.GetCount() == 1)
				{
					m_Textures.push_back({ name, nullptr });
				}
				else if (resourceDecl.GetCount() > 1)
				{
					std::vector<Ref<Texture2D>> textures(resourceDecl.GetCount());
					m_TextureArrays.push_back({ name, textures });
				}
			}
		}
	}
}