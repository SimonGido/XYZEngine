#pragma once
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/PipelineCompute.h"

#include "XYZ/Asset/Asset.h"
#include "XYZ/Core/Core.h"

#include "ShaderAsset.h"
namespace XYZ {

	class XYZ_API MaterialAsset : public Asset
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
		// Regular textures
		void SetTexture(const std::string& name, Ref<Texture2D> texture);
		// Texture arrays
		void SetTexture(const std::string& name, Ref<Texture2D> texture, uint32_t index);

		// NOTE: unused
		void SetOpaque(bool opaque) { m_Opaque = opaque; }
		bool IsOpaque() const { return m_Opaque; }
		/////////////////

		template <typename T>
		void Set(const std::string_view name, const T& val);

		template <typename T>
		T&	 Get(const std::string_view name);

		template <typename T>
		void Specialize(const std::string& name, T value);

		Ref<ShaderAsset>	  GetShaderAsset()	    const { return m_ShaderAsset; }
		Ref<Shader>			  GetShader()			const { return m_Material->GetShader(); }
		Ref<Material>		  GetMaterial()			const { return m_Material; }
		Ref<MaterialInstance> GetMaterialInstance() const { return m_MaterialInstance; }
		
		const std::vector<TextureData>&		 GetTextures()	    const { return m_Textures; }
		const std::vector<TextureArrayData>& GetTextureArrays() const { return m_TextureArrays; }
		const PipelineSpecialization&		 GetSpecialization() const { return m_Specialization; }

		virtual AssetType GetAssetType() const override { return GetStaticType(); }
		static AssetType GetStaticType() { return AssetType::Material; }
	private:
		void setup();
		void setupTextureBuffers();
		void applyTexturesToMaterial();
		void setupSpecialization();

		TextureData*	  findTextureData(const std::string& name);
		TextureArrayData* findTextureArrayData(const std::string& name);

	private:
		Ref<ShaderAsset>			  m_ShaderAsset;
		Ref<Material>				  m_Material;
		Ref<MaterialInstance>		  m_MaterialInstance;
		std::vector<TextureData>	  m_Textures;
		std::vector<TextureArrayData> m_TextureArrays;
		bool						  m_Opaque = true; // NOTE: unused

		// NOTE: this is used only in compute pipeline now
		PipelineSpecialization m_Specialization;
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
		return m_MaterialInstance->Get<T>(name);
	}

	template<typename T>
	inline void MaterialAsset::Specialize(const std::string& name, T value)
	{
		Ref<Shader> shader = m_ShaderAsset->GetShader();
		const auto& cache = shader->GetSpecializationCachce();
		auto it = cache.find(name);
		XYZ_ASSERT(it != cache.end(), "Specialization with name {} does not exist", name);
		m_Specialization.Set(name, value);
	}

}