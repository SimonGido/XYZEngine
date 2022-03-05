#include "stdafx.h"
#include "OpenGLShader.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Utils/StringUtils.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>


#include <fstream>
#include <array>
#include <filesystem>

namespace XYZ {
	
	namespace Utils {

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			XYZ_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			case GL_COMPUTE_SHADER: return "GL_COMPUTE_SHADER";
			}
			XYZ_ASSERT(false, "");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Assets/Cache/Shader/Opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			}
			XYZ_ASSERT(false, "");
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			XYZ_ASSERT(false, "");
			return "";
		}
	}

	static GLenum ShaderComponentFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		if (type == "geometry")
			return GL_GEOMETRY_SHADER;
		if (type == "compute")
			return GL_COMPUTE_SHADER;

		XYZ_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	std::string readShaderFromFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			XYZ_ASSERT(false, "Could not load shader!");
		}
		return result;
	}


	OpenGLShader::OpenGLShader(const std::string& path)
		: 
		m_Name(Utils::GetFilenameWithoutExtension(path)),
		m_NumTakenTexSlots(0), 
		m_AssetPath(path)
	{
		Utils::CreateCacheDirectoryIfNeeded();
		Reload();
	}
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& path)
		: m_Name(name), m_AssetPath(path)
	{
		Utils::CreateCacheDirectoryIfNeeded();
		Reload();
	}
	OpenGLShader::~OpenGLShader()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteProgram(rendererID);
		});
	}

	void OpenGLShader::Bind() const
	{
		Ref<const OpenGLShader> instance = this;
		Renderer::Submit([instance]() {
			glUseProgram(instance->m_RendererID); 
			});
	}
	void OpenGLShader::Compute(uint32_t groupX, uint32_t groupY, uint32_t groupZ, ComputeBarrierType barrierType) const
	{
		XYZ_ASSERT(m_IsCompute, "Calling compute on non compute shader");
		Ref<const OpenGLShader> instance = this;
		Renderer::Submit([instance, groupX, groupY, groupZ, barrierType]() {
			glDispatchCompute(groupX, groupY, groupZ);
			switch (barrierType)
			{
			case XYZ::ComputeBarrierType::ShaderStorageBarrier:
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				break;
			case XYZ::ComputeBarrierType::ShaderImageAccessBarrier:
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				break;
			default:
				break;
			}
			
		});
	}
	void OpenGLShader::Unbind() const
	{
		Renderer::Submit([=]() {
			glUseProgram(0);
			});
	}

	void OpenGLShader::SetVSUniforms(ByteBuffer buffer) const
	{
		uint32_t counter = 0;
		for (auto& uniform : m_VSUniformList.Uniforms)
		{
			if (uniform.GetCount() > 1)
			{
				Renderer::Submit([=]() {
					setUniformArr(&uniform, m_VSUniformLocations[counter], buffer);
					});
			}
			else
			{
				Renderer::Submit([=]() {
					setUniform(&uniform, m_VSUniformLocations[counter], buffer);
					});
			}
			counter++;
		}
	}

	void OpenGLShader::SetFSUniforms(ByteBuffer buffer)const
	{
		uint32_t counter = 0;
		for (auto& uniform : m_FSUniformList.Uniforms)
		{
			if (uniform.GetCount() > 1)
			{
				Renderer::Submit([=]() {
					setUniformArr(&uniform, m_FSUniformLocations[counter], buffer);
					});
			}
			else
			{
				Renderer::Submit([=]() {
					setUniform(&uniform, m_FSUniformLocations[counter], buffer);
					});
			}
			counter++;
		}
	}


	void OpenGLShader::setUniform(const ShaderUniform* uniform, uint32_t location, ByteBuffer data) const
	{
		switch (uniform->GetDataType())
		{
		case ShaderUniformDataType::Float:
			uploadFloat(location, *(float*)& data[uniform->GetOffset()]);
			break;
		case ShaderUniformDataType::Vec2:
			uploadFloat2(location, *(glm::vec2*) & data[uniform->GetOffset()]);
			break;
		case ShaderUniformDataType::Vec3:
			uploadFloat3(location, *(glm::vec3*) & data[uniform->GetOffset()]);
			break;
		case ShaderUniformDataType::Vec4:
			uploadFloat4(location, *(glm::vec4*) & data[uniform->GetOffset()]);
			break;
		case ShaderUniformDataType::Int:
			uploadInt(location, *(int*)& data[uniform->GetOffset()]);
			break;
		case ShaderUniformDataType::Mat4:
			uploadMat4(location, *(glm::mat4*) & data[uniform->GetOffset()]);
			break;
		};
	}

	void OpenGLShader::setUniformArr(const ShaderUniform* uniform, uint32_t location, ByteBuffer data) const
	{
		switch (uniform->GetDataType())
		{
		case ShaderUniformDataType::Float:
			uploadFloatArr(location, (float*)& data[uniform->GetOffset()], uniform->GetCount());
			break;
		case ShaderUniformDataType::Vec2:
			uploadFloat2Arr(location, *(glm::vec2*) & data[uniform->GetOffset()], uniform->GetCount());
			break;
		case ShaderUniformDataType::Vec3:
			uploadFloat3Arr(location, *(glm::vec3*) & data[uniform->GetOffset()], uniform->GetCount());
			break;
		case ShaderUniformDataType::Vec4:
			uploadFloat4Arr(location, *(glm::vec4*) & data[uniform->GetOffset()], uniform->GetCount());
			break;
		case ShaderUniformDataType::Int:
			uploadIntArr(location, (int*)& data[uniform->GetOffset()], uniform->GetCount());
			break;
		case ShaderUniformDataType::Mat4:
			uploadMat4Arr(location, *(glm::mat4*) & data[uniform->GetOffset()], uniform->GetCount());
			break;
		};
	}


	void OpenGLShader::Reload(bool forceCompile)
	{
		const std::string source = readFile(m_AssetPath);
		preProcess(source);
		
		compileOrGetVulkanBinaries();
		compileOrGetOpenGLBinaries();
		Ref<OpenGLShader> instance = this;
		Renderer::SubmitAndWait([instance]() mutable {	
	
			instance->createProgram();
		});
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat(location, value);
		});
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat2(location, value);
			});
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat3(location, value);
			});
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat4(location, value);
			});
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadInt(location, value);
			});
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			const auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadMat4(location, value);
			});
	}

	void OpenGLShader::uploadInt(uint32_t loc, int value) const
	{
		glUniform1i(loc, value);
	}

	void OpenGLShader::uploadFloat(uint32_t loc, float value) const
	{
		glUniform1f(loc, value);
	}
	void OpenGLShader::uploadFloat2(uint32_t loc, const glm::vec2& value) const
	{
		glUniform2f(loc, value.x, value.y);
	}
	void OpenGLShader::uploadFloat3(uint32_t loc, const glm::vec3& value) const
	{
		glUniform3f(loc, value.x, value.y, value.z);
	}
	void OpenGLShader::uploadFloat4(uint32_t loc, const glm::vec4& value) const
	{
		glUniform4f(loc, value.x, value.y, value.z, value.w);
	}
	void OpenGLShader::uploadMat3(uint32_t loc, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::uploadMat4(uint32_t loc, const glm::mat4& matrix) const
	{		
		glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)glm::value_ptr(matrix));
	}
	void OpenGLShader::uploadIntArr(uint32_t loc, int* values, uint32_t count) const
	{
		glUniform1iv(loc, count, values);
	}
	void OpenGLShader::uploadFloatArr(uint32_t loc, float* values, uint32_t count) const
	{
		glUniform1fv(loc, count, values);
	}
	void OpenGLShader::uploadFloat2Arr(uint32_t loc, const glm::vec2& value, uint32_t count) const
	{
		glUniform2fv(loc, count, (float*)& value);
	}
	void OpenGLShader::uploadFloat3Arr(uint32_t loc, const glm::vec3& value, uint32_t count) const
	{
		glUniform3fv(loc, count, (float*)& value);
	}
	void OpenGLShader::uploadFloat4Arr(uint32_t loc, const glm::vec4& value, uint32_t count) const
	{
		glUniform4fv(loc, count, (float*)& value);
	}
	void OpenGLShader::uploadMat3Arr(uint32_t loc, const glm::mat3& matrix, uint32_t count) const
	{
		glUniformMatrix3fv(loc, count, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::uploadMat4Arr(uint32_t loc, const glm::mat4& matrix, uint32_t count) const
	{
		glUniformMatrix4fv(loc, count, GL_FALSE, glm::value_ptr(matrix));
	}
	


	void OpenGLShader::preProcess(const std::string& source)
	{
		const char* TypeToken = "#type";
		const size_t TypeTokenLength = strlen(TypeToken);
		size_t pos = source.find(TypeToken, 0);
		while (pos != std::string::npos)
		{
			const size_t eol = source.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			const size_t begin = pos + TypeTokenLength + 1;
			std::string Type = source.substr(begin, eol - begin);
			XYZ_ASSERT(ShaderComponentFromString(Type), "Invalid shader Component specified");

			const size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(TypeToken, nextLinePos);
			m_ShaderSources[ShaderComponentFromString(Type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
			if (Type == "compute")
				m_IsCompute = true;
		}
	}
	
	void OpenGLShader::resolveUniforms()
	{
		glUseProgram(m_RendererID);
		m_VSUniformLocations.resize(m_VSUniformList.Uniforms.size());
		m_FSUniformLocations.resize(m_FSUniformList.Uniforms.size());
		uint32_t counter = 0;
		for (auto& uni : m_VSUniformList.Uniforms)
		{
			m_VSUniformLocations[counter++] = glGetUniformLocation(m_RendererID, uni.GetName().c_str());
		}
		counter = 0;
		for (auto& uni : m_FSUniformList.Uniforms)
		{
			m_FSUniformLocations[counter++] = glGetUniformLocation(m_RendererID, uni.GetName().c_str());
		}
	}
	
	size_t OpenGLShader::GetHash() const
	{
		return std::hash<std::string>{}(m_AssetPath);
	}

	void OpenGLShader::reflect(unsigned int stage, const std::vector<uint32_t>& shaderData)
	{
		
	}

	void OpenGLShader::createProgram()
	{
		const GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			XYZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_AssetPath, infoLog.data());

			glDeleteProgram(program);

			for (const auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (const auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::compileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		
	}

	void OpenGLShader::compileOrGetVulkanBinaries()
	{
		
	}

	std::string OpenGLShader::readFile(const std::string& filepath) const
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			const size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				XYZ_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			XYZ_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}
}