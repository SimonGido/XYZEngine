#pragma once
#include "Asset.h"

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/Shader.h"

namespace XYZ {

	class ShaderAsset : public Asset
	{
	public:
		ShaderAsset(Ref<Shader> shader);
		ShaderAsset(const std::string& filepath, size_t sourceHash, std::vector<BufferLayout> layouts);
		ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash, std::vector<BufferLayout> layouts);

		void SetLayouts(std::vector<BufferLayout> layouts);

		Ref<Shader> GetShader() const { return m_Shader; }
		size_t	    GetSourceHash() const { return m_SourceHash; }
		const std::vector<BufferLayout>& GetLayouts() const { return m_Layouts; }
	
		static AssetType GetStaticType() { return AssetType::Shader; }
	private:
		std::vector<BufferLayout> m_Layouts;
		Ref<Shader>				  m_Shader;
		size_t					  m_SourceHash;
	};
}