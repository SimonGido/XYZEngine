#include "stdafx.h"
#include "Material.h"


namespace XYZ {
	
	Material::Material(const Ref<Shader>& shader)
	{
		m_Shader = shader;
		m_Buffer.Allocate(shader->GetUniformSize());
		m_Buffer.ZeroInitialize();
		m_Shader->AddReloadCallback(std::bind(&Material::OnShaderReload, this));
		m_Key = m_Shader->GetRendererID();
	}

	Material::~Material()
	{
		delete[] m_Buffer;
	}

	void Material::Bind()
	{
		m_Shader->Bind();
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			auto& texture = m_Textures[i];
			if (texture)
				texture->Bind(i);
		}

		m_Shader->SetUniforms(m_Buffer);
		m_Shader->UploadRoutines();
	}

	void Material::OnShaderReload()
	{
		delete[] m_Buffer;
		m_Buffer.Allocate(m_Shader->GetUniformSize());
		m_Key = m_Shader->GetRendererID();

		for (auto& it : m_MaterialInstances)
			it->OnShaderReload();
	}

	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		: m_Material(material)
	{
		m_Material->m_MaterialInstances.insert(this);
		m_Buffer = ByteBuffer::Copy(m_Material->m_Buffer, m_Material->m_Shader->GetUniformSize());
	}

	MaterialInstance::~MaterialInstance()
	{
		m_Material->m_MaterialInstances.erase(this);
		delete[] m_Buffer;
	}

	void MaterialInstance::Bind()
	{
		m_Material->m_Shader->SetUniforms(m_Buffer);
	}

	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material)
	{
		return Ref<MaterialInstance>::Create(material);
	}

	void MaterialInstance::OnShaderReload()
	{
		delete[] m_Buffer;
		m_Buffer.Allocate(m_Material->m_Shader->GetUniformSize());
		m_Buffer.ZeroInitialize();
	}

	void MaterialInstance::UpdateMaterialValue(const Uniform* uni)
	{
		if (m_UpdatedValues.find(uni->Name) == m_UpdatedValues.end())
		{
			auto data = (uint8_t*)m_Material->m_Buffer + uni->Offset;
			m_Buffer.Write(data, uni->Size, uni->Offset);
		}
	}

}