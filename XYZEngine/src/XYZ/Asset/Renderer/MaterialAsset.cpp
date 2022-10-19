#include "stdafx.h"
#include "MaterialAsset.h"

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {

	MaterialAsset::MaterialAsset(Ref<ShaderAsset> shaderAsset)
		:
		m_ShaderAsset(shaderAsset)
	{
		setup();
	}
	MaterialAsset::~MaterialAsset()
	{
	}

	void MaterialAsset::SetShaderAsset(Ref<ShaderAsset> shaderAsset)
	{
		XYZ_ASSERT(!IsFlagSet(AssetFlag::ReadOnly), "Asset is readonly");
		m_ShaderAsset = shaderAsset;
		setup();
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
	void MaterialAsset::setup()
	{
		Ref<Shader> shader = m_ShaderAsset->GetShader();
		m_Material = Material::Create(shader);
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);

		// We want weak ref so Material does not keep MaterialAsset alive
		WeakRef<MaterialAsset> instance = this;
		m_Material->m_OnInvalidate = [instance]() mutable {
			if (instance->IsValid())
			{
				instance->setupTextureBuffers();
				instance->applyTexturesToMaterial();
				instance->setupSpecialization();
			}
		};

		setupTextureBuffers();
	}
	void MaterialAsset::setupTextureBuffers()
	{
		auto& resources = m_ShaderAsset->GetShader()->GetResources();
		Ref<Texture2D> whiteTexture = Renderer::GetDefaultResources().RendererAssets.at("WhiteTexture").As<Texture2D>();
		

		std::vector<TextureData>	  newTextures;
		std::vector<TextureArrayData> newTextureArrays;

		for (auto& [name, resourceDecl] : resources)
		{
			if (resourceDecl.GetType() == ShaderResourceType::Sampler2D)
			{
				if (resourceDecl.GetCount() == 1)
				{
					// Check if texture was stored in old data
					TextureData* oldData = findTextureData(name);
					if (oldData != nullptr)
						newTextures.push_back(*oldData);
					else
						newTextures.push_back({ name, whiteTexture });
				}
				else if (resourceDecl.GetCount() > 1)
				{
					std::vector<Ref<Texture2D>> textures(resourceDecl.GetCount());
					
					// Check if texture was stored in old data
					TextureArrayData* oldData = findTextureArrayData(name);
					for (size_t i = 0; i < textures.size(); ++i)
					{
						if (oldData != nullptr && oldData->Textures.size() > i)
							textures[i] = oldData->Textures[i];
						else
							textures[i] = whiteTexture;
					}
					newTextureArrays.push_back({ name, textures });
				}
			}
		}
		m_Textures = std::move(newTextures);
		m_TextureArrays = std::move(newTextureArrays);
	}
	void MaterialAsset::applyTexturesToMaterial()
	{
		for (auto& it : m_Textures)
		{
			m_Material->SetImage(it.Name, it.Texture->GetImage());
		}

		for (auto& texArray : m_TextureArrays)
		{
			uint32_t index = 0;
			for (auto& texture : texArray.Textures)
			{
				m_Material->SetImageArray(texArray.Name, texture->GetImage(), index);
				index++;
			}
		}
	}
	void MaterialAsset::setupSpecialization()
	{
		PipelineSpecialization newSpecialization;
		Ref<Shader> shader = m_ShaderAsset->GetShader();
		const auto& cache = shader->GetSpecializationCachce();
		for (auto& spec : m_Specialization.GetValues())
		{
			auto it = cache.find(spec.Name);
			if (it != cache.end())
			{
				newSpecialization.Set(spec.Name, spec.Data);
			}
		}
		m_Specialization = std::move(newSpecialization);
	}
	MaterialAsset::TextureData* MaterialAsset::findTextureData(const std::string& name)
	{
		for (auto& texture : m_Textures)
		{
			if (texture.Name == name)
				return &texture;
		}
		return nullptr;
	}
	MaterialAsset::TextureArrayData* MaterialAsset::findTextureArrayData(const std::string& name)
	{
		for (auto& texture : m_TextureArrays)
		{
			if (texture.Name == name)
				return &texture;
		}
		return nullptr;
	}
}