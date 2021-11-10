#include "stdafx.h"
#include "Shader.h"

#include "Renderer.h"
#include "XYZ/API/OpenGL/OpenGLShader.h"


namespace XYZ {
	size_t Shader::GetHash() const
	{
		return std::hash<std::string>{}(FilePath);
	}
	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		case RendererAPI::API::OpenGL:
		{
			return  Ref<OpenGLShader>::Create(path);
		}
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	Ref<Shader> Shader::Create(const std::string& name, const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		case RendererAPI::API::OpenGL:
		{
			return Ref<OpenGLShader>::Create(name, path);
		}
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		if (m_Shaders.find(name) == m_Shaders.end())
		{
			m_Shaders[name] = shader;
		}
		else XYZ_ASSERT(false, "Shader name already exists.");
	}
	Ref<Shader> ShaderLibrary::Load(const std::string& path)
	{
		auto shader = Shader::Create(path);
		Add(shader);
		return shader;
	}
	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		auto shader = Shader::Create(name, path);
		Add(shader);
		return shader;
	}
	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		if (Exists(name)) return m_Shaders[name];
		XYZ_ASSERT(false, "Shader does not exist.");
		return Ref<Shader>();
	}
	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
	size_t ShaderVariable::Size() const
	{
		return SizeOfUniformType(Type);
	}
	size_t ShaderStruct::Size() const
	{
		size_t result = 0;
		for (auto& var : Variables)
			result += var.Size();
		return result;
	}
	ShaderUniform::ShaderUniform(std::string name, ShaderUniformDataType dataType, uint32_t size, uint32_t offset, uint32_t count, uint32_t loc)
		:
		m_Name(std::move(name)),
		m_DataType(dataType),
		m_Size(size),
		m_Offset(offset),
		m_Count(count),
		m_Location(loc)
	{
	}
}