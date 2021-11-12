#pragma once
#include "XYZ/Renderer/Shader.h"

#include <vulkan/vulkan.h>

namespace XYZ {

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& path);
		VulkanShader(const std::string& name, const std::string& path);
		
		virtual void Reload(bool forceCompile = false) override;

	private:
		void reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);
		void compileOrGetVulkanBinaries(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outputBinary, bool forceCompile);
		void createProgram(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void preProcess(const std::string& source);
		std::string readFile(const std::string& filepath) const;

	private:
		std::string m_Name;
		std::string m_AssetPath;
		std::unordered_map<VkShaderStageFlagBits, std::string> m_ShaderSources;
		std::vector<Shader::ReloadCallback>					   m_ShaderReloadCallbacks;

		std::vector<VkPipelineShaderStageCreateInfo>		   m_PipelineShaderStageCreateInfos;
	};
}