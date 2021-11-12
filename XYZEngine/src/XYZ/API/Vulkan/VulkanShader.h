#pragma once
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/ShaderResource.h"

#include <vulkan/vulkan.h>

namespace XYZ {

	class VulkanShader : public Shader
	{
	public:
		struct PushConstantRange
		{
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t Offset = 0;
			uint32_t Size = 0;
		};

	public:
		VulkanShader(const std::string& path);
		VulkanShader(const std::string& name, const std::string& path);
		virtual ~VulkanShader() override;

		virtual void Reload(bool forceCompile = false) override;
		virtual void AddReloadCallback(Shader::ReloadCallback callback) override;

		inline virtual const std::string& GetPath() const override { return m_AssetPath; };
		inline virtual const std::string& GetName() const override { return m_Name; }

		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
		const std::vector<PushConstantRange>&	  GetPushConstantRanges() const { return m_PushConstantRanges; }
	private:
		void reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);
		void compileOrGetVulkanBinaries(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outputBinary, bool forceCompile);
		void createProgram(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void preProcess(const std::string& source);
		std::string readFile(const std::string& filepath) const;

	private:
		std::string m_Name;
		std::string m_AssetPath;
		
		std::unordered_map<VkShaderStageFlagBits, std::string>     m_ShaderSources;
		std::vector<VkPipelineShaderStageCreateInfo>			   m_PipelineShaderStageCreateInfos;
		std::vector<VkDescriptorSetLayout>						   m_DescriptorSetLayouts;

		std::unordered_map<std::string, ShaderResourceDeclaration> m_Resources;
		std::vector<PushConstantRange>							   m_PushConstantRanges;
		std::vector<Shader::ReloadCallback>					       m_ShaderReloadCallbacks;
		std::unordered_map<std::string, ShaderBuffer>			   m_Buffers;
	};
}