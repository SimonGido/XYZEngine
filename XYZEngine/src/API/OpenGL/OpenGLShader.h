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
		virtual void Unbind() const override;
		virtual void Compute(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1) const override;
		virtual void SetVSUniforms(ByteBuffer buffer) override;
		virtual void SetFSUniforms(ByteBuffer buffer) override;


		virtual void Reload() override;
		virtual void AddReloadCallback(std::function<void()> callback) override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const UniformList& GetVSUniformList() const override { return m_VSUniformList; }
		virtual const UniformList& GetFSUniformList() const override { return m_FSUniformList; }
			
		inline virtual const std::string& GetPath() const override { return m_AssetPath; };
		inline virtual const std::string& GetName() const override { return m_Name; }

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
	private:
		void parse();
		void load(const std::string& source);
		void parseUniform(const std::string& statement, ShaderType type);
		void compileAndUpload();
		void resolveUniforms();

		std::unordered_map<uint32_t, std::string> preProcess(const std::string& source);

		void parseSource(unsigned int component,const std::string& source);

		void setUniform(const Uniform* uniform, ByteBuffer data);
		void setUniformArr(const Uniform* uniform, ByteBuffer data);

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
		std::string m_AssetPath;

		uint32_t m_NumTakenTexSlots;
		
		UniformList m_VSUniformList;
		UniformList m_FSUniformList;

		TextureUniformList m_TextureList;

		std::vector<std::function<void()>> m_ShaderReloadCallbacks;
		std::unordered_map<uint32_t, std::string> m_ShaderSources;

		// Temporary, in future we will get that information from the GPU
		static constexpr uint32_t sc_MaxTextureSlots = 32;
	};

}