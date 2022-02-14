#include "stdafx.h"
#include "MaterialAsset.h"


namespace XYZ {

	MaterialAsset::MaterialAsset(Ref<Shader> shader)
	{
		m_Material = Material::Create(shader);
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);
	}
	MaterialAsset::~MaterialAsset()
	{
	}
	void MaterialAsset::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		for (auto& it : m_Textures)
		{
			if (it.Name == name)
			{
				it.Texture = texture;
				m_Material->SetImage(it.Name, texture->GetImage());
				return;
			}
		}
		std::string nameStr = std::string(name);
		m_Textures.push_back({ nameStr, texture });
		m_Material->SetImage(nameStr, texture->GetImage());
	}
	void MaterialAsset::SetTexture(const std::string& name, Ref<Texture2D> texture, uint32_t index)
	{
		const size_t arrayIndex = static_cast<size_t>(index);
		for (auto& texArray : m_TextureArrays)
		{
			if (texArray.Name == name)
			{
				if (texArray.Textures.size() <= arrayIndex)
					texArray.Textures.resize(arrayIndex + 1);

				texArray.Textures[index] = texture;
				m_Material->SetImageArray(texArray.Name, texture->GetImage(), index);
				return;
			}
		}
		m_TextureArrays.push_back({ name, {} });
		auto& last = m_TextureArrays.back();
		if (last.Textures.size() <= arrayIndex)
			last.Textures.resize(arrayIndex + 1);

		last.Textures[arrayIndex] = texture;
		m_Material->SetImage(name, texture->GetImage(), index);
	}
}