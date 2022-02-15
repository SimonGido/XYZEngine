#pragma once
#include "Asset.h"

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/Shader.h"

namespace XYZ {

	class ShaderAsset : public Asset
	{
	public:
		ShaderAsset(const std::string& filepath, std::vector<BufferLayout> layouts);

		void SetLayouts(std::vector<BufferLayout> layouts);


		Ref<Shader> GetShader() const { return m_Shader; }
		const std::vector<BufferLayout>& GetLayouts() const { return m_Layouts; }
	
	private:
		std::vector<BufferLayout> m_Layouts;
		Ref<Shader>				  m_Shader;
	};
}