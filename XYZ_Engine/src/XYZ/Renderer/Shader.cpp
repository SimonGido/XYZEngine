#include "stdafx.h"
#include "Shader.h"

#include "Renderer.h"
#include "API/OpenGL/OpenGLShader.h"


namespace XYZ {
	std::shared_ptr<Shader> Shader::Create(const std::string& path)
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
			return std::make_shared<OpenGLShader>(path);
		}
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	std::shared_ptr<Shader> Shader::Create(const std::string& name, const std::string& path)
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
			return std::make_shared<OpenGLShader>(name, path);
		}
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	void ShaderLibrary::Add(const std::shared_ptr<Shader>& shader)
	{
		auto& name = shader->GetName();
		if (m_Shaders.find(name) == m_Shaders.end())
		{
			m_Shaders[name] = shader;
		}
		else XYZ_ASSERT(false, "Shader name already exists.");
	}
	std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& path)
	{
		auto shader = Shader::Create(path);
		Add(shader);
		return shader;
	}
	std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		auto shader = Shader::Create(name, path);
		Add(shader);
		return shader;
	}
	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name)
	{
		if (Exists(name)) return m_Shaders[name];
		XYZ_ASSERT(false, "Shader does not exist.");
		return std::shared_ptr<Shader>();
	}
	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
}