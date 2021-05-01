#include "stdafx.h"
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <array>
#include <GL/glew.h>

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {

	UniformDataType StringToType(const std::string& type)
	{
		if (type == "int")			return UniformDataType::Int;
		if (type == "float")		return UniformDataType::Float;
		if (type == "vec2")			return UniformDataType::Vec2;
		if (type == "vec3")			return UniformDataType::Vec3;
		if (type == "vec4")			return UniformDataType::Vec4;
		if (type == "mat3")			return UniformDataType::Mat3;
		if (type == "mat4")			return UniformDataType::Mat4;
		if (type == "sampler2D")	return UniformDataType::Sampler2D;
		return UniformDataType::None;
	}
	uint32_t SizeOfUniformType(UniformDataType type)
	{
		switch (type)
		{
		case UniformDataType::Int:        return 4;
		case UniformDataType::Float:      return 4;
		case UniformDataType::Vec2:       return 4 * 2;
		case UniformDataType::Vec3:       return 4 * 3;
		case UniformDataType::Vec4:       return 4 * 4;
		case UniformDataType::Mat3:       return 4 * 3 * 3;
		case UniformDataType::Mat4:       return 4 * 4 * 4;
		}
		return 0;
	}

	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
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

	std::vector<std::string> SplitString(const std::string& string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	std::vector<std::string> Tokenize(const std::string& string)
	{
		return SplitString(string, " \t\n");
	}

	std::vector<std::string> GetLines(const std::string& string)
	{
		return SplitString(string, "\n");
	}

	std::string GetStatement(const char* str, const char** outPosition)
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
		: m_NumTakenTexSlots(0), m_AssetPath(path)
	{
		Reload();
	}
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& path)
		: m_Name(name), m_AssetPath(path)
	{
		Reload();
	}
	OpenGLShader::~OpenGLShader()
	{
		Renderer::Submit([=]() {
			glDeleteProgram(m_RendererID); 
			});
	}
	void OpenGLShader::Bind() const
	{
		Renderer::Submit([=]() {
			glUseProgram(m_RendererID); 
			});
	}
	void OpenGLShader::Compute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) const
	{
		XYZ_ASSERT(m_IsCompute, "Calling compute on non compute shader");
		Renderer::Submit([=]() {
			glDispatchCompute(groupX, groupY, groupZ);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
		std::string source = readShaderFromFile(m_AssetPath);
		load(source);

		for (size_t i = 0; i < m_ShaderReloadCallbacks.size(); ++i)
			m_ShaderReloadCallbacks[i]();
	}

	void OpenGLShader::AddReloadCallback(std::function<void()> callback)
	{
		m_ShaderReloadCallbacks.push_back(callback);
	}


	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadFloat(location, value);
		});
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadFloat2(location, value);
			});
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadFloat3(location, value);
			});
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadFloat4(location, value);
			});
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadInt(location, value);
			});
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		Renderer::Submit([=]() {
			auto location = glGetUniformLocation(m_RendererID, name.c_str());
			XYZ_ASSERT(location != -1, "Uniform ", name, " does not exist");
			uploadMat4(location, value);
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
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
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
	
	void OpenGLShader::parseUniform(const std::string& statement, ShaderType type)
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

		auto dataType = StringToType(typeString);
		auto size = SizeOfUniformType(dataType);
		if (dataType != UniformDataType::None)
		{			
			if (dataType == UniformDataType::Sampler2D)
			{
				m_TextureList.Textures.push_back(TextureUniform{ name, m_TextureList.Count, count });
				m_TextureList.Count += count;
			}
			else if (type == ShaderType::Vertex)
			{
				Uniform uniform{
					   name, dataType, type, m_VSUniformList.Size, size, count, 0
				};
				m_VSUniformList.Uniforms.push_back(uniform);
				m_VSUniformList.Size += size * count;
			}
			else
			{
				Uniform uniform{
					   name, dataType, type, m_FSUniformList.Size, size, count, 0
				};
				m_FSUniformList.Uniforms.push_back(uniform);
				m_FSUniformList.Size += size * count;
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

				XYZ_LOG_ERR(infoLog.data());
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


			XYZ_LOG_ERR(infoLog.data());
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
		//for (auto& texture : m_TextureList.Textures)
		//{
		//	if (texture.Count > 1)
		//	{	
		//		int32_t location = glGetUniformLocation(m_RendererID, texture.Name.c_str());
		//		int* samplers = new int[texture.Count];
		//		for (uint32_t s = 0; s < texture.Count; ++s)
		//			samplers[s] = texture.Slot + s;
		//		glUniform1iv(location, texture.Count, samplers);
		//		delete[] samplers;
		//	}
		//	else
		//	{
		//		int32_t location = glGetUniformLocation(m_RendererID, texture.Name.c_str());
		//		glUniform1i(location, texture.Slot);
		//	}
		//}
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
	
	void OpenGLShader::parse()
	{
		const char* token;
		const char* vstr;
		const char* fstr;


		m_VSUniformList.Uniforms.clear();
		m_VSUniformList.Size = 0;
		m_FSUniformList.Uniforms.clear();
		m_VSUniformList.Size = 0;

		auto& vertexSource = m_ShaderSources[GL_VERTEX_SHADER];
		auto& fragmentSource = m_ShaderSources[GL_FRAGMENT_SHADER];

		// Vertex Shader
		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "uniform"))
			parseUniform(GetStatement(token, &vstr), ShaderType::Vertex);

		// Fragment Shader
		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "uniform"))
			parseUniform(GetStatement(token, &fstr), ShaderType::Fragment);

		if (m_IsCompute)
		{
			const char* cstr = m_ShaderSources[GL_COMPUTE_SHADER].c_str();
			while (token = FindToken(cstr, "uniform"))
				parseUniform(GetStatement(token, &cstr), ShaderType::Vertex);
		}
	}

}