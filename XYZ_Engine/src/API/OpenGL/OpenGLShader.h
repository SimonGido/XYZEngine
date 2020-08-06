#pragma once
#include "XYZ/Renderer/Shader.h"

namespace XYZ {
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& path);
		OpenGLShader(const std::string& name, const std::string& path);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Compute(unsigned int groupX, unsigned int groupY = 1, unsigned int groupZ = 1) const override;
		virtual void Unbind() const override;
		virtual void SetUniforms(unsigned char* data) override;
		virtual void SetSubRoutine(const std::string& name) override;
		virtual void UploadRoutines() override;

		virtual void Reload() override;
		virtual void Recompile() override;
		virtual void AddReloadCallback(std::function<void()> callback) override;
		virtual void AddSource(const std::string& filepath) override;

		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const Uniform* FindUniform(const std::string& name) const override;
		virtual const TextureUniform* FindTexture(const std::string& name) const override;

		inline virtual std::string GetPath() const override { return m_Path; };
		inline virtual std::string GetName() const override { return m_Name; }

		virtual uint32_t GetUniformSize() const { return m_UniformsSize; };
		virtual const std::vector<Uniform>& GetUniforms() const override { return m_Uniforms; };
	private:
		std::string readFile(const std::string& filepath);
		std::unordered_map<unsigned int, std::string> preProcess(const std::string& source);
		void compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
		void parseUniforms();
		void parseSubRoutines();
		void addUniform(UniformDataType type, unsigned int size, unsigned int offset, const std::string& name, unsigned int count = 0);

		void parseSource(unsigned int Component,const std::string& source);

		void setUniform(Uniform* uniform, unsigned char* data);
		void setUniformArr(Uniform* uniform, unsigned char* data);

		void uploadInt(uint32_t loc, int value);
		void uploadFloat(uint32_t loc, float value);
		void uploadFloat2(uint32_t loc, const glm::vec2& value);
		void uploadFloat3(uint32_t loc, const glm::vec3& value);
		void uploadFloat4(uint32_t loc, const glm::vec4& value);
		void uploadMat3(uint32_t loc, const glm::mat3& matrix);
		void uploadMat4(uint32_t loc, const glm::mat4& matrix);

		void uploadIntArr(uint32_t loc, int* values, uint32_t count);
		void uploadFloatArr(uint32_t loc, float* values, uint32_t count);
		void uploadFloat2Arr(uint32_t loc, const glm::vec2& value, uint32_t count);
		void uploadFloat3Arr(uint32_t loc, const glm::vec3& value, uint32_t count);
		void uploadFloat4Arr(uint32_t loc, const glm::vec4& value, uint32_t count);
		void uploadMat3Arr(uint32_t loc, const glm::mat3& matrix, uint32_t count);
		void uploadMat4Arr(uint32_t loc, const glm::mat4& matrix, uint32_t count);
	private:
		uint32_t m_RendererID = 0;
		ShaderProgramType m_Type;

		std::string m_Name;
		std::string m_Path;

		unsigned int m_NumTakenTexSlots;
		unsigned int m_UniformsSize;

		std::vector<Uniform> m_Uniforms;
		std::vector<TextureUniform> m_Textures;
		std::vector<Routine> m_Routines;

		std::vector<std::function<void()>> m_ShaderReloadCallbacks;
		std::unordered_map<unsigned int, std::string> m_ShaderSources;

		// Temporary, in future we will get that information from the GPU
		static constexpr int sc_MaxTextureSlots = 32;
	};

}