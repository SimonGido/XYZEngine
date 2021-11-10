#include "stdafx.h"
#include "OpenGLShader.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Utils/StringUtils.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

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

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			XYZ_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			XYZ_ASSERT(false, "");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
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

	static std::string GetEndStruct(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, "};");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		size_t length = end - str + 1;
		return std::string(str, length);
	}
	
	static ShaderStruct ParseStruct(const std::string& structSource)
	{
		ShaderStruct shaderStruct;
		std::vector<std::string> tokens = std::move(Utils::SplitString(structSource, "\t\n"));
		std::vector<std::string> structName = std::move(Utils::SplitString(tokens[0], " \r"));
		shaderStruct.Name = structName[1];
		for (size_t i = 1; i < tokens.size(); ++i)
		{
			std::vector<std::string> variables = std::move(Utils::SplitString(tokens[i], " \r"));
			if (variables.size() > 1)
			{
				UniformDataType type = StringToShaderDataType(variables[0]);
				variables[1].pop_back(); // pop ;
				std::string name = variables[1];
				shaderStruct.Variables.push_back({ name, type });
			}
		}
		return shaderStruct;
	}

	static std::vector<ShaderStruct> ParseStructs(const std::string& source)
	{
		std::vector<ShaderStruct> structs;
		const char* token = nullptr;
		const char* src = source.c_str();
		while (token = Utils::FindToken(src, "struct"))
		{
			structs.push_back(ParseStruct(GetEndStruct(token, &src)));
		}
		return structs;
	}


	static std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
	{
		size_t start = 0;
		size_t end = string.find_first_of(delimiters);

		std::vector<std::string> result;

		while (end <= std::string::npos)
		{
			std::string token = string.substr(start, end - start);
			if (!token.empty())
				result.push_back(token);

			if (end == std::string::npos)
				break;

			start = end + 1;
			end = string.find_first_of(delimiters, start);
		}

		return result;
	}

	static std::vector<std::string> SplitString(const std::string& string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	static std::vector<std::string> Tokenize(const std::string& string)
	{
		return SplitString(string, " \t\n");
	}

	static std::vector<std::string> GetLines(const std::string& string)
	{
		return SplitString(string, "\n");
	}

	static std::string GetStatement(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, ";");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string(str, length);
	}

	static std::string CutArrayIndexing(const std::string& name)
	{
		std::string str;
		int count = 0;
		while (name[count] != '[')
			count++;

		str = name.substr(0, count);
		return str;
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
		for (auto& uniform : m_VSUniformList.Uniforms)
		{
			if (uniform.Count > 1)
			{
				Renderer::Submit([=]() {
					setUniformArr(&uniform, buffer);
					});
			}
			else
			{
				Renderer::Submit([=]() {
					setUniform(&uniform, buffer);
					});
			}
		}
	}

	void OpenGLShader::SetFSUniforms(ByteBuffer buffer)const
	{
		for (auto& uniform : m_FSUniformList.Uniforms)
		{
			if (uniform.Count > 1)
			{
				Renderer::Submit([=]() {
					setUniformArr(&uniform, buffer);
					});
			}
			else
			{
				Renderer::Submit([=]() {
					setUniform(&uniform, buffer);
					});
			}
		}
	}

	
	void OpenGLShader::parseSource(uint32_t component,const std::string& source)
	{		
		const char* versionToken = "#version";
		size_t versionTokenLength = strlen(versionToken);
		size_t verPos = m_ShaderSources[component].find(versionToken, 0);
		size_t sourcePos = verPos + versionTokenLength + 1;
		size_t sourceEol = source.find_first_of("\r\n", sourcePos);

		const char* ComponentToken = "#type";
		size_t ComponentTokenLength = strlen(ComponentToken);
		size_t pos = source.find(ComponentToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + ComponentTokenLength + 1;

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(ComponentToken, nextLinePos);
			m_ShaderSources[component].insert(sourceEol + 2 ,source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos)));
		}
	}

	void OpenGLShader::setUniform(const Uniform* uniform, ByteBuffer data) const
	{
		switch (uniform->DataType)
		{
		case UniformDataType::Float:
			uploadFloat(uniform->Location, *(float*)& data[uniform->Offset]);
			break;
		case UniformDataType::Vec2:
			uploadFloat2(uniform->Location, *(glm::vec2*) & data[uniform->Offset]);
			break;
		case UniformDataType::Vec3:
			uploadFloat3(uniform->Location, *(glm::vec3*) & data[uniform->Offset]);
			break;
		case UniformDataType::Vec4:
			uploadFloat4(uniform->Location, *(glm::vec4*) & data[uniform->Offset]);
			break;
		case UniformDataType::Int:
			uploadInt(uniform->Location, *(int*)& data[uniform->Offset]);
			break;
		case UniformDataType::Mat4:
			uploadMat4(uniform->Location, *(glm::mat4*) & data[uniform->Offset]);
			break;
		};
	}

	void OpenGLShader::setUniformArr(const Uniform* uniform, ByteBuffer data) const
	{
		switch (uniform->DataType)
		{
		case UniformDataType::Float:
			uploadFloatArr(uniform->Location, (float*)& data[uniform->Offset], uniform->Count);
			break;
		case UniformDataType::Vec2:
			uploadFloat2Arr(uniform->Location, *(glm::vec2*) & data[uniform->Offset], uniform->Count);
			break;
		case UniformDataType::Vec3:
			uploadFloat3Arr(uniform->Location, *(glm::vec3*) & data[uniform->Offset], uniform->Count);
			break;
		case UniformDataType::Vec4:
			uploadFloat4Arr(uniform->Location, *(glm::vec4*) & data[uniform->Offset], uniform->Count);
			break;
		case UniformDataType::Int:
			uploadIntArr(uniform->Location, (int*)& data[uniform->Offset], uniform->Count);
			break;
		case UniformDataType::Mat4:
			uploadMat4Arr(uniform->Location, *(glm::mat4*) & data[uniform->Offset], uniform->Count);
			break;
		};
	}


	void OpenGLShader::Reload()
	{	
		std::string source = readFile(m_AssetPath);
		m_ShaderSources = preProcess(source);

		compileOrGetVulkanBinaries();
		compileOrGetOpenGLBinaries();

		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance]() mutable {		
			instance->createProgram();
		});

		for (size_t i = 0; i < m_ShaderReloadCallbacks.size(); ++i)
			m_ShaderReloadCallbacks[i]();
	}

	void OpenGLShader::AddReloadCallback(std::function<void()> callback)
	{
		m_ShaderReloadCallbacks.emplace_back(std::move(callback));
	}


	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat(location, value);
		});
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat2(location, value);
			});
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat3(location, value);
			});
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadFloat4(location, value);
			});
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			instance->uploadInt(location, value);
			});
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance, name, value]() {
			auto location = glGetUniformLocation(instance->m_RendererID, name.c_str());
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
	
	void OpenGLShader::load(const std::string& source)
	{
		m_ShaderSources = preProcess(source);
		parse();

		Ref<OpenGLShader> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->compileAndUpload();
			instance->resolveUniforms();
		});
	}

	std::unordered_map<uint32_t, std::string> OpenGLShader::preProcess(const std::string& source)
	{
		const char* TypeToken = "#type";
		size_t TypeTokenLength = strlen(TypeToken);
		size_t pos = source.find(TypeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + TypeTokenLength + 1;
			std::string Type = source.substr(begin, eol - begin);
			XYZ_ASSERT(ShaderComponentFromString(Type), "Invalid shader Component specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(TypeToken, nextLinePos);
			m_ShaderSources[ShaderComponentFromString(Type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
			if (Type == "compute")
				m_IsCompute = true;
		}
		return m_ShaderSources;
	}
	
	void OpenGLShader::parseUniform(const std::string& statement, ShaderType type, const std::vector<ShaderStruct>& structs)
	{
		std::vector<std::string> tokens = Tokenize(statement);
		uint32_t index = 0;

		index++; // "uniform"
		std::string typeString = tokens[index++];
		std::string name = tokens[index++];
		// Strip ; from name if present
		if (const char* s = strstr(name.c_str(), ";"))
			name = std::string(name.c_str(), s - name.c_str());

		std::string n(name);
		uint32_t count = 1;
		const char* namestr = n.c_str();
		if (const char* s = strstr(namestr, "["))
		{
			name = std::string(namestr, s - namestr);
			const char* end = strstr(namestr, "]");
			std::string c(s + 1, end - s);
			count = atoi(c.c_str());
		}

		UniformDataType dataType = StringToShaderDataType(typeString);
		uint32_t size = SizeOfUniformType(dataType);
		
		UniformList* targetList = nullptr;
		if (type == ShaderType::Vertex || type == ShaderType::Compute)
			targetList = &m_VSUniformList;
		else
			targetList = &m_FSUniformList;

		if (dataType != UniformDataType::None)
		{			
			if (dataType == UniformDataType::Sampler2D)
			{
				m_TextureList.Textures.push_back(TextureUniform{ name, m_TextureList.Count, count });
				m_TextureList.Count += count;
			}
			else
			{
				Uniform uniform{
					   name, dataType, type, targetList->Size, size, count, 0
				};
				targetList->Uniforms.push_back(uniform);
				targetList->Size += size * count;
			}
		}
		else
		{		
			for (auto& structType : structs)
			{
				if (structType.Name == typeString)
				{
					for (auto& var : structType.Variables)
					{
						size = SizeOfUniformType(var.Type);
						Uniform uniform{
							name + "." + var.Name, var.Type, type, targetList->Size, size, count, 0
						};
						targetList->Uniforms.push_back(uniform);
						targetList->Size += size * count;
					}
					break;
				}
			}			
		}
	}
	void OpenGLShader::compileAndUpload()
	{
		GLuint program = glCreateProgram();
		XYZ_ASSERT(m_ShaderSources.size() <= 3, "We only support 3 shaders for now");
		std::array<GLenum, 3> glShaderIDs;

		int glShaderIDIndex = 0;
		for (auto& kv : m_ShaderSources)
		{
			if (kv.second.empty())
				continue;

			GLenum Component = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(Component);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				XYZ_CORE_ERROR(infoLog.data());
				XYZ_ASSERT(false, "Shader compilation failure!");
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}
		if (m_RendererID)
			glDeleteProgram(m_RendererID);

		// Link our program
		m_RendererID = program;
		glLinkProgram(m_RendererID);

		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);

		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_RendererID);

			for (size_t i = 0; i < glShaderIDIndex; ++i)
				glDeleteShader(glShaderIDs[i]);


			XYZ_CORE_ERROR(infoLog.data());
			XYZ_ASSERT(false, "Shader link failure!");
			return;
		}

		for (size_t i = 0; i < glShaderIDIndex; ++i)
		{
			glDetachShader(m_RendererID, glShaderIDs[i]);
			glDeleteShader(glShaderIDs[i]);
		}
	}
	void OpenGLShader::resolveUniforms()
	{
		glUseProgram(m_RendererID);
		for (auto& uni : m_VSUniformList.Uniforms)
		{
			uni.Location = glGetUniformLocation(m_RendererID, uni.Name.c_str());
		}
		for (auto& uni : m_FSUniformList.Uniforms)
		{
			uni.Location = glGetUniformLocation(m_RendererID, uni.Name.c_str());
		}
	}
	const char* FindToken(const char* str, const std::string& token)
	{
		const char* t = str;
		while (t = strstr(t, token.c_str()))
		{
			bool left = str == t || isspace(t[-1]);
			bool right = !t[token.size()] || isspace(t[token.size()]);
			if (left && right)
				return t;
			t += token.size();
		}
		return nullptr;
	}
	
	void OpenGLShader::reflect(unsigned int stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		XYZ_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_AssetPath);
		XYZ_TRACE("		{0} uniform buffers", resources.uniform_buffers.size());
		XYZ_TRACE("		{0} resources", resources.sampled_images.size());

		XYZ_TRACE("Uniform buffers:");

		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize    = compiler.get_declared_struct_size(bufferType);
			uint32_t binding	   = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount		   = bufferType.member_types.size();

			XYZ_TRACE("  {0}", resource.name);
			XYZ_TRACE("    Size = {0}", bufferSize);
			XYZ_TRACE("    Binding = {0}", binding);
			XYZ_TRACE("    Members = {0}", memberCount);
		}
		for (const auto& resource : resources.sampled_images)
		{
			XYZ_TRACE("  {0}", resource.name);
			auto& baseType = compiler.get_type(resource.base_type_id);
			auto& type = compiler.get_type(resource.type_id);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = baseType.image.dim;
			uint32_t arraySize = type.array[0];
		}
	}

	void OpenGLShader::createProgram()
	{
		GLuint program = glCreateProgram();

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
			XYZ_ERROR("Shader linking failed ({0}):\n{1}", m_AssetPath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::compileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_AssetPath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					XYZ_ERROR(module.GetErrorMessage());
					XYZ_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::compileOrGetVulkanBinaries()
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : m_ShaderSources)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_AssetPath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					XYZ_ERROR(module.GetErrorMessage());
					XYZ_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			reflect(stage, data);
	}

	std::string OpenGLShader::readFile(const std::string& filepath) const
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				XYZ_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			XYZ_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	void OpenGLShader::parse()
	{
		const char* token;
		const char* vstr;
		const char* fstr;

		m_TextureList.Textures.clear();
		m_TextureList.Count = 0;
		m_VSUniformList.Uniforms.clear();
		m_VSUniformList.Size = 0;
		m_FSUniformList.Uniforms.clear();
		m_VSUniformList.Size = 0;

		auto& vertexSource = m_ShaderSources[GL_VERTEX_SHADER];
		auto& fragmentSource = m_ShaderSources[GL_FRAGMENT_SHADER];

		std::vector<ShaderStruct> vertexStructs = std::move(ParseStructs(vertexSource));
		std::vector<ShaderStruct> fragmentStructs = std::move(ParseStructs(fragmentSource));

		// Vertex Shader
		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "uniform"))
			parseUniform(GetStatement(token, &vstr), ShaderType::Vertex, vertexStructs);

		// Fragment Shader
		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "uniform"))
			parseUniform(GetStatement(token, &fstr), ShaderType::Fragment, fragmentStructs);

		if (m_IsCompute)
		{
			const char* cstr = m_ShaderSources[GL_COMPUTE_SHADER].c_str();
			std::vector<ShaderStruct> computeStructs = std::move(ParseStructs(cstr));
			while (token = FindToken(cstr, "uniform"))
				parseUniform(GetStatement(token, &cstr), ShaderType::Vertex, computeStructs);
		}
	}

}