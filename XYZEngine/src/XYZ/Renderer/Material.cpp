#include "stdafx.h"
#include "Material.h"



namespace XYZ {
	
	Material::Material(const Ref<Shader>& shader)
		:
		m_Flags(RenderFlags::MaterialFlag),
		m_RenderQueueID(0)
	{
		m_Shader = shader;
		m_VSUniformBuffer.Allocate(m_Shader->GetVSUniformList().Size);
		m_FSUniformBuffer.Allocate(m_Shader->GetFSUniformList().Size);

		m_Shader->AddReloadCallback(std::bind(&Material::onShaderReload, this));
	}

	Material::~Material()
	{
		delete[] m_VSUniformBuffer;
		delete[] m_FSUniformBuffer;
	}

	void Material::SetTexture(const std::string& name, const Ref<Texture>& texture, uint32_t index)
	{
		auto tex = findTexture(name);
		XYZ_ASSERT(tex, "Material texture does not exist ", name.c_str());

		if ((uint32_t)m_Textures.size() <= tex->Slot + index)
			m_Textures.resize((size_t)tex->Slot + 1 + index);

		m_Textures[size_t(tex->Slot) + size_t(index)] = texture;
	}

	bool Material::HasProperty(const std::string& name) const
	{
		auto [uni, type] = findUniform(name);
		return type != ShaderType::None;
	}

	void Material::Bind() const
	{
		m_Shader->Bind();
		for (uint32_t i = 0; i < (uint32_t)m_Textures.size(); i++)
		{
			if (m_Textures[i].Raw())
				m_Textures[i]->Bind(i);
		}

		if (m_VSUniformBuffer)
			m_Shader->SetVSUniforms(m_VSUniformBuffer);
		if (m_FSUniformBuffer)
			m_Shader->SetFSUniforms(m_FSUniformBuffer);
	}


	bool Material::operator==(const Material& other) const
	{
		return m_Shader->GetRendererID() == other.m_Shader->GetRendererID()
			&& m_RenderQueueID == other.m_RenderQueueID
			&& m_Flags == other.m_Flags;
	}

	bool Material::operator!=(const Material& other) const
	{
		return m_Shader->GetRendererID() == other.m_Shader->GetRendererID()
			&& m_RenderQueueID == other.m_RenderQueueID
			&& m_Flags == other.m_Flags;
	}

	void Material::onShaderReload()
	{
		m_VSUniformBuffer.Allocate(m_Shader->GetVSUniformList().Size);
		m_FSUniformBuffer.Allocate(m_Shader->GetFSUniformList().Size);
	}

	ByteBuffer& Material::getUniformBufferTarget(ShaderType type)
	{
		if (type == ShaderType::Vertex)
			return m_VSUniformBuffer;
		return m_FSUniformBuffer;
	}

	std::pair<const ShaderUniform*, ShaderType> Material::findUniform(const std::string& name) const
	{
		for (auto& uni : m_Shader->GetVSUniformList().Uniforms)
		{
			if (uni.GetName() == name)
				return { &uni, ShaderType::Vertex };
		}
		for (auto& uni : m_Shader->GetFSUniformList().Uniforms)
		{
			if (uni.GetName() == name)
				return { &uni, ShaderType::Fragment };
		}
		return { nullptr, ShaderType::None };
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
		: 
		Material(material->GetShader()),
		m_Material(material)
	{
	}

	MaterialInstance::~MaterialInstance()
	{
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
}