#pragma once
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/ShaderResource.h"
#include "XYZ/Renderer/Material.h"

#include "Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"


#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace XYZ {

	class VulkanShader : public Shader
	{
	public:
		struct UniformBuffer
		{
			uint32_t Size = 0;
			uint32_t BindingPoint = 0;
			std::string Name;
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct StorageBuffer
		{
			VmaAllocation MemoryAlloc = nullptr;
			uint32_t Size = 0;
			uint32_t BindingPoint = 0;
			std::string Name;
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct ImageSampler
		{
			uint32_t BindingPoint = 0;
			uint32_t DescriptorSet = 0;
			uint32_t ArraySize = 0;
			std::string Name;
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
		struct PushConstantRange
		{
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t Offset = 0;
			uint32_t Size = 0;
		};

		struct ShaderDescriptorSet
		{
			std::unordered_map<uint32_t, UniformBuffer*> UniformBuffers;
			std::unordered_map<uint32_t, StorageBuffer*> StorageBuffers;
			std::unordered_map<uint32_t, ImageSampler>   ImageSamplers;
			std::unordered_map<uint32_t, ImageSampler>   StorageImages;

			std::unordered_map<std::string, VkWriteDescriptorSet> WriteDescriptorSets;

			operator bool() const { return !(StorageBuffers.empty() && UniformBuffers.empty() && ImageSamplers.empty() && StorageImages.empty()); }
		};

		struct Set
		{
			VkDescriptorSetLayout			  DescriptorSetLayout;
			ShaderDescriptorSet				  ShaderDescriptorSet;
			std::vector<VkDescriptorPoolSize> DescriptorPools;
		};

		struct ShaderMaterialDescriptorSet
		{
			std::vector<VkDescriptorSet> DescriptorSets;
		};

		struct ShaderBufferWriteDescription
		{
			// Per set
			std::vector<VkDescriptorSet>	  DescriptorSets;
			std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
			std::vector<uint32_t>			  Bindings;
		};

	public:
		VulkanShader(const std::string& path);
		VulkanShader(const std::string& name, const std::string& path);
		virtual ~VulkanShader() override;

		virtual void Reload(bool forceCompile = false) override;
		virtual void AddReloadCallback(Shader::ReloadCallback callback) override;

		inline virtual const std::string& GetPath() const override { return m_AssetPath; };
		inline virtual const std::string& GetName() const override { return m_Name; }


		ShaderMaterialDescriptorSet						   AllocateDescriptorSet(uint32_t set = 0);
	
		//const VkWriteDescriptorSet&						   GetUniformBufferWriteDescriptorSet(uint32_t binding, uint32_t frame) const;
		const std::vector<Set>&							   GetDescriptorSets() const { return m_DescriptorSets; }
		const std::vector<PushConstantRange>&			   GetPushConstantRanges()	 const { return m_PushConstantRanges; }
		const std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos() const { return m_PipelineShaderStageCreateInfos; }
		const std::vector<ShaderBufferWriteDescription>&   GetUniformBufferWriteDescriptions() const { return m_UniformBufferWriteDescriptions; }
		std::vector<VkDescriptorSetLayout>				   GetAllDescriptorSetLayouts() const;
		
	private:
		void compileOrGetVulkanBinaries(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outputBinary, bool forceCompile);
		void createProgram(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void preProcess(const std::string& source);
		
		void reflectAllStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);
		void reflectStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);
		void reflectConstantBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers);
		void reflectStorageBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers);
		void reflectUniformBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers);
		void reflectSampledImages(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& sampledImages);
		void reflectStorageImages(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& storageImages);


		void createUniformBufferWriteDescriptions();
		void createDescriptorSetLayout();
		void createDescriptorPools();

		void destroy();	
		void onReload();


	private:
		std::string		 m_Name;
		std::string		 m_AssetPath;
		std::vector<Set> m_DescriptorSets;

		std::vector<ShaderBufferWriteDescription>					m_UniformBufferWriteDescriptions;

		std::unordered_map<VkShaderStageFlagBits, std::string>		m_ShaderSources;
		std::vector<VkPipelineShaderStageCreateInfo>				m_PipelineShaderStageCreateInfos;

		std::unordered_map<std::string, ShaderResourceDeclaration>	m_Resources;
		std::vector<PushConstantRange>								m_PushConstantRanges;

		std::vector<Shader::ReloadCallback>							m_ShaderReloadCallbacks;
		std::unordered_map<std::string, ShaderBuffer>				m_Buffers;
	};
}