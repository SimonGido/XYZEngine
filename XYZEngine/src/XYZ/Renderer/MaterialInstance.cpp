#include "stdafx.h"	
#include "MaterialInstance.h"
#include "Material.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		:
		m_Material(material)
	{
		m_Material->m_MaterialInstances.insert(this);
		allocateStorage(m_Material->GetShader()->GetBuffers());
	}

	MaterialInstance::~MaterialInstance()
	{
		m_Material->m_MaterialInstances.erase(this);
	}

	Ref<Shader> MaterialInstance::GetShader() const
	{
		return m_Material->GetShader();
	}

	const ByteBuffer MaterialInstance::GetFSUniformsBuffer() const
	{
		const size_t vertexBufferSize = m_Material->GetShader()->GetVertexBufferSize();
		if (m_UniformsBuffer.Size - vertexBufferSize != 0)
			return ByteBuffer(&m_UniformsBuffer.Data[vertexBufferSize], m_UniformsBuffer.Size - vertexBufferSize);
		return ByteBuffer(nullptr, 0);
	}

	const ByteBuffer MaterialInstance::GetVSUniformsBuffer() const
	{
		const size_t vertexBufferSize = m_Material->GetShader()->GetVertexBufferSize();
		return ByteBuffer(m_UniformsBuffer.Data, vertexBufferSize);
	}

	void MaterialInstance::allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers) const
	{
		uint32_t size = 0;
		for (auto [name, shaderBuffer] : buffers)
			size += shaderBuffer.Size;

		if (m_UniformsBuffer.Size != size)
		{
			if (m_UniformsBuffer)
				m_UniformsBuffer.Destroy();
			if (size)
			{
				m_UniformsBuffer.Allocate(size);
				m_UniformsBuffer.ZeroInitialize();
			}
		}
	}

	std::pair<const ShaderUniform*, ByteBuffer*> MaterialInstance::findUniformDeclaration(const std::string_view name)
	{
		std::string bufferName = std::string(Utils::FirstSubString(name, '.'));
		const auto& shaderBuffers = m_Material->GetShader()->GetBuffers();
		auto itBuffer = shaderBuffers.find(bufferName);
		if (itBuffer != shaderBuffers.end())
		{
			const ShaderBuffer& buffer = itBuffer->second;
			auto itUniform = buffer.Uniforms.find(std::string(name));
			if (itUniform != buffer.Uniforms.end())
				return { &itUniform->second, &m_UniformsBuffer };
		}
		return { nullptr, nullptr };
	}
}