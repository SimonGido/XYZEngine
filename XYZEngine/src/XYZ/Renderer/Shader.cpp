#include "stdafx.h"
#include "Shader.h"

#include "Renderer.h"
#include "XYZ/API/OpenGL/OpenGLShader.h"
#include "XYZ/API/Vulkan/VulkanShader.h"

namespace XYZ {
	
	Ref<Shader> Shader::Create(const std::string& path, size_t sourceHash, bool forceCompile)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		case RendererAPI::Type::OpenGL: return  Ref<OpenGLShader>::Create(path);
		case RendererAPI::Type::Vulkan: return Ref<VulkanShader>::Create(path, sourceHash, forceCompile);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	Ref<Shader> Shader::Create(const std::string& name, const std::string& path, size_t sourceHash, bool forceCompile)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		case RendererAPI::Type::OpenGL: return Ref<OpenGLShader>::Create(name, path);
		case RendererAPI::Type::Vulkan: return Ref<VulkanShader>::Create(name, path, sourceHash, forceCompile);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, size_t sourceHash, bool forceCompile)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		//case RendererAPI::Type::OpenGL: return Ref<OpenGLShader>::Create(name, path);
		case RendererAPI::Type::Vulkan: return Ref<VulkanShader>::Create(name, vertexPath, fragmentPath, sourceHash, forceCompile);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	

	ShaderUniform::ShaderUniform(std::string name, ShaderUniformDataType dataType, uint32_t size, uint32_t offset, uint32_t count)
		:
		m_Name(std::move(name)),
		m_DataType(dataType),
		m_Size(size),
		m_Offset(offset),
		m_Count(count)
	{
	}
}