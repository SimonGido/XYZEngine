#pragma once
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Material.h"

#include "XYZ/Asset/Asset.h"
#include "XYZ/Core/Core.h"

#include "ShaderAsset.h"

namespace XYZ {

	class MaterialAsset : public Asset
	{
	public:
		struct TextureData
		{
			std::string	   Name;
			Ref<Texture2D> Texture;
		};
		struct TextureArrayData
		{
			std::string Name;
			std::vector<Ref<Texture2D>> Textures;
		};
	public:
		MaterialAsset(Ref<ShaderAsset> shaderAsset);
		virtual ~MaterialAsset() override;

		void SetShaderAsset(Ref<ShaderAsset> shaderAsset);
		void SetTexture(const std::string& name, Ref<Texture2D> texture);
		void SetTexture(const std::string& name, Ref<Texture2D> texture, uint32_t index);

		template <typename T>
		void Set(const std::string_view name, const T& val);

		template <typename T>
		T&	 Get(const std::string_view name);

		Ref<ShaderAsset>	  GetShaderAsset()	    const { return m_ShaderAsset; }
		Ref<Shader>			  GetShader()			const { return m_Material->GetShader(); }
		Ref<Material>		  GetMaterial()			const { return m_Material; }
		Ref<MaterialInstance> GetMaterialInstance() const { return m_MaterialInstance; }
		
		const std::vector<TextureData>&		 GetTextures()	    const { return m_Textures; }
		const std::vector<TextureArrayData>& GetTextureArrays() const { return m_TextureArrays; }

		virtual AssetType GetAssetType() const override { return GetStaticType(); }
		static AssetType GetStaticType() { return AssetType::Material; }
	private:
		void setupTextureBuffers();

	private:
		Ref<ShaderAsset>			  m_ShaderAsset;
		Ref<Material>				  m_Material;
		Ref<MaterialInstance>		  m_MaterialInstance;
		std::vector<TextureData>	  m_Textures;
		std::vector<TextureArrayData> m_TextureArrays;
		// TODO: Probably flags
	};

	template<typename T>
	inline void MaterialAsset::Set(const std::string_view name, const T& val)
	{
		m_MaterialInstance->Set(name, val);
	}

	template<typename T>
	inline T& MaterialAsset::Get(const std::string_view name)
	{
		return m_MaterialInstance->Get(name);
	}

}