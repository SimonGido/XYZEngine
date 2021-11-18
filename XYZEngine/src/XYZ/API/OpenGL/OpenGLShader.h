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
		virtual void Compute(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1, ComputeBarrierType barrierType = ComputeBarrierType::ShaderStorageBarrier) const override;
		virtual void SetVSUniforms(ByteBuffer buffer) const override;
		virtual void SetFSUniforms(ByteBuffer buffer) const override;


		virtual void Reload(bool forceCompile = false) override;
	
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const UniformList&		  GetVSUniformList() const override { return m_VSUniformList; }
		virtual const UniformList&		  GetFSUniformList() const override { return m_FSUniformList; }
		virtual const TextureUniformList& GetTextureList() const override { return m_TextureList; }

		inline virtual const std::string& GetPath() const override { return m_AssetPath; };
		inline virtual const std::string& GetName() const override { return m_Name; }
		virtual size_t					  GetHash() const override;
		virtual uint32_t				  GetRendererID() const override { return m_RendererID; }
	private:
		void reflect(unsigned int stage, const std::vector<uint32_t>& shaderData);
		void createProgram();
		void resolveUniforms();
		void compileOrGetOpenGLBinaries();
		void compileOrGetVulkanBinaries();
		void preProcess(const std::string& source);
		std::string readFile(const std::string& filepath) const;
	

		void setUniform(const ShaderUniform* uniform, uint32_t location, ByteBuffer data) const;
		void setUniformArr(const ShaderUniform* uniform, uint32_t location, ByteBuffer data) const;

		void uploadInt (uint32_t loc, int value) const;
		void uploadFloat(uint32_t loc, float value) const;
		void uploadFloat2(uint32_t loc, const glm::vec2& value) const;
		void uploadFloat3(uint32_t loc, const glm::vec3& value) const;
		void uploadFloat4(uint32_t loc, const glm::vec4& value) const;
		void uploadMat3(uint32_t loc, const glm::mat3& matrix) const;
		void uploadMat4(uint32_t loc, const glm::mat4& matrix) const;

		void uploadIntArr(uint32_t loc, int* values, uint32_t count) const;
		void uploadFloatArr(uint32_t loc, float* values, uint32_t count) const;
		void uploadFloat2Arr(uint32_t loc, const glm::vec2& value, uint32_t count) const;
		void uploadFloat3Arr(uint32_t loc, const glm::vec3& value, uint32_t count) const;
		void uploadFloat4Arr(uint32_t loc, const glm::vec4& value, uint32_t count) const;
		void uploadMat3Arr(uint32_t loc, const glm::mat3& matrix, uint32_t count) const;
		void uploadMat4Arr(uint32_t loc, const glm::mat4& matrix, uint32_t count) const;
	private:
		uint32_t m_RendererID = 0;
		bool m_IsCompute = false;
	
		std::string m_Name;
		std::string m_AssetPath;

		uint32_t m_NumTakenTexSlots;
		
		std::vector<uint32_t> m_VSUniformLocations;
		std::vector<uint32_t> m_FSUniformLocations;
		UniformList		   m_VSUniformList;
		UniformList		   m_FSUniformList;
		TextureUniformList m_TextureList;


		std::unordered_map<uint32_t, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<uint32_t, std::vector<uint32_t>> m_OpenGLSPIRV;
		std::unordered_map<uint32_t, std::string>			m_OpenGLSourceCode;
		std::unordered_map<uint32_t, std::string>		    m_ShaderSources;

		// Temporary, in future we will get that information from the GPU
		static constexpr uint32_t sc_MaxTextureSlots = 32;
	};

}