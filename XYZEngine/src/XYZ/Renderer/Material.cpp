#include "stdafx.h"
#include "Material.h"



namespace XYZ {
	
	Material::Material(const Ref<Shader>& shader)
	{
		m_Shader = shader;
		m_VSUniformBuffer.Allocate(m_Shader->GetVSUniformList().Size);
		m_FSUniformBuffer.Allocate(m_Shader->GetFSUniformList().Size);

		m_Shader->AddReloadCallback(std::bind(&Material::onShaderReload, this));
		m_Flags = m_Shader->GetRendererID();
	}

	Material::~Material()
	{
		delete[] m_VSUniformBuffer;
		delete[] m_FSUniformBuffer;
	}			

	void Material::Bind() const
	{
		m_Shader->Bind();
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			if (m_Textures[i].Raw())
				m_Textures[i]->Bind(i);
		}

		if (m_VSUniformBuffer)
			m_Shader->SetVSUniforms(m_VSUniformBuffer);
		if (m_FSUniformBuffer)
			m_Shader->SetFSUniforms(m_FSUniformBuffer);
	}


	void Material::onShaderReload()
	{
		delete[] m_VSUniformBuffer;
		delete[] m_FSUniformBuffer;

		m_VSUniformBuffer.Allocate(m_Shader->GetVSUniformList().Size);
		m_FSUniformBuffer.Allocate(m_Shader->GetFSUniformList().Size);
		m_Flags = m_Shader->GetRendererID();

		for (auto& it : m_MaterialInstances)
			it->onShaderReload();
	}

	ByteBuffer& Material::getUniformBufferTarget(ShaderType type)
	{
		if (type == ShaderType::Vertex)
			return m_VSUniformBuffer;
		return m_FSUniformBuffer;
	}

	const Uniform* Material::findUniform(const std::string& name) const
	{
		for (auto& uni : m_Shader->GetVSUniformList().Uniforms)
		{
			if (uni.Name == name)
				return &uni;
		}
		for (auto& uni : m_Shader->GetFSUniformList().Uniforms)
		{
			if (uni.Name == name)
				return &uni;
		}
		return nullptr;
	}

	const TextureUniform* Material::findTexture(const std::string& name) const
	{
		for (auto& uni : m_Shader->GetTextureList().Textures)
		{
			if (uni.Name == name)
				return &uni;
		}
		for (auto& uni : m_Shader->GetTextureList().Textures)
		{
			if (uni.Name == name)
				return &uni;
		}
		return nullptr;
	}

	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		: m_Material(material)
	{
		m_Material->m_MaterialInstances.insert(this);
		m_VSUniformBuffer = ByteBuffer::Copy(m_Material->m_VSUniformBuffer, m_Material->m_VSUniformBuffer.GetSize());
		m_FSUniformBuffer = ByteBuffer::Copy(m_Material->m_FSUniformBuffer, m_Material->m_FSUniformBuffer.GetSize());
	}

	MaterialInstance::~MaterialInstance()
	{
		m_Material->m_MaterialInstances.erase(this);
		delete[] m_VSUniformBuffer;
		delete[] m_FSUniformBuffer;
	}


	void MaterialInstance::Bind() const
	{
		if (m_VSUniformBuffer)
			m_Material->m_Shader->SetVSUniforms(m_VSUniformBuffer);
		if (m_FSUniformBuffer)
			m_Material->m_Shader->SetFSUniforms(m_FSUniformBuffer);
	}

	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material)
	{
		return Ref<MaterialInstance>::Create(material);
	}

	void MaterialInstance::onShaderReload()
	{
		delete[] m_VSUniformBuffer;
		delete[] m_FSUniformBuffer;

		m_VSUniformBuffer.Allocate(m_Material->m_Shader->GetVSUniformList().Size);
		m_FSUniformBuffer.Allocate(m_Material->m_Shader->GetFSUniformList().Size);
	}

	void MaterialInstance::updateMaterialValue(const Uniform* uni)
	{
		if (m_UpdatedValues.find(uni->Name) == m_UpdatedValues.end())
		{
			if (uni->ShaderType == ShaderType::Vertex)
			{
				auto data = (uint8_t*)m_Material->m_VSUniformBuffer + uni->Offset;
				m_VSUniformBuffer.Write(data, uni->Size, uni->Offset);
			}
			else
			{
				auto data = (uint8_t*)m_Material->m_FSUniformBuffer + uni->Offset;
				m_FSUniformBuffer.Write(data, uni->Size, uni->Offset);
			}
		}
	}

	ByteBuffer& MaterialInstance::getUniformBufferTarget(ShaderType type)
	{
		if (type == ShaderType::Vertex)
			return m_VSUniformBuffer;
		return m_FSUniformBuffer;
	}

}