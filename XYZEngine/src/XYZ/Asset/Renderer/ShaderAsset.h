#pragma once
#include "XYZ/Asset/Asset.h"

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/Shader.h"

namespace XYZ {

	class XYZ_API ShaderAsset : public Asset
	{
	public:
		ShaderAsset(Ref<Shader> shader);
		ShaderAsset(const std::string& filepath, size_t sourceHash);
		ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash);
		~ShaderAsset();

		virtual AssetType GetAssetType() const override { return AssetType::Shader; }

		Ref<Shader> GetShader() const { return m_Shader; }
		size_t	    GetSourceHash() const;

		static AssetType GetStaticType() { return AssetType::Shader; }
	private:
		Ref<Shader>	m_Shader;
	};
}