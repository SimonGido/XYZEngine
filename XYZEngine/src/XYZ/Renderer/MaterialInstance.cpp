#include "stdafx.h"	
#include "MaterialInstance.h"
#include "Material.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {

	MaterialInstance::~MaterialInstance()
	{
	}

	bool MaterialInstance::HasUniform(const std::string_view name) const
	{
		auto decl = findUniformDeclaration(name);
		if (!decl)
			return false;
		return true;
	}

	Ref<Shader> MaterialInstance::GetShader() const
	{
		return m_Material->GetShader();
	}

	PushConstBuffer MaterialInstance::GetFSUniformsBuffer() const
	{
		PushConstBuffer result;
		const size_t vertexBufferSize = m_Material->GetShader()->GetVertexBufferSize();		
		if (m_UniformsBuffer.Size - vertexBufferSize != 0)
		{
			result.Size = m_UniformsBuffer.Size - vertexBufferSize;
			memcpy(result.Bytes, &m_UniformsBuffer.Bytes[vertexBufferSize], result.Size);
		}
		return result;
	}

	PushConstBuffer MaterialInstance::GetVSUniformsBuffer() const
	{
		PushConstBuffer result;
		const size_t vertexBufferSize = m_Material->GetShader()->GetVertexBufferSize();
		memcpy(result.Bytes, m_UniformsBuffer.Bytes, vertexBufferSize);
		result.Size = vertexBufferSize;
		return result;
	}

	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		:
		m_Material(material)
	{
		const auto& buffers = m_Material->GetShader()->GetBuffers();
		initializeStorage(buffers);
	}


	void MaterialInstance::initializeStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers)
	{
		uint32_t size = 0;
		for (auto [name, shaderBuffer] : buffers)
			size += shaderBuffer.Size;

		m_UniformsBuffer = {};
		m_UniformsBuffer.Size = size;
	}

	const ShaderUniform* MaterialInstance::findUniformDeclaration(const std::string_view name)
	{
		std::string bufferName = std::string(Utils::FirstSubString(name, '.'));
		const auto& shaderBuffers = m_Material->GetShader()->GetBuffers();
		auto itBuffer = shaderBuffers.find(bufferName);
		if (itBuffer != shaderBuffers.end())
		{
			const ShaderBuffer& buffer = itBuffer->second;
			auto itUniform = buffer.Uniforms.find(std::string(name));
			if (itUniform != buffer.Uniforms.end())
				return &itUniform->second;
		}
		return nullptr;
	}
	const ShaderUniform* MaterialInstance::findUniformDeclaration(const std::string_view name) const
	{
		std::string bufferName = std::string(Utils::FirstSubString(name, '.'));
		const auto& shaderBuffers = m_Material->GetShader()->GetBuffers();
		auto itBuffer = shaderBuffers.find(bufferName);
		if (itBuffer != shaderBuffers.end())
		{
			const ShaderBuffer& buffer = itBuffer->second;
			auto itUniform = buffer.Uniforms.find(std::string(name));
			if (itUniform != buffer.Uniforms.end())
				return &itUniform->second;
		}
		return nullptr;
	}
}