#pragma once
#include "XYZ/Renderer/Pipeline.h"
#include "Vulkan.h"
#include "VulkanShader.h"
#include <map>

namespace XYZ {
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& specs);
		virtual ~VulkanPipeline() override;

		virtual void Invalidate() override;
		virtual void SetUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t set = 0) override;

		virtual PipelineSpecification& GetSpecification() override { return m_Specification; }
		virtual const PipelineSpecification& GetSpecification() const override { return m_Specification; }

		VkPipelineLayout GetVulkanPipelineLayout() const { return m_PipelineLayout; }
		VkPipeline GetVulkanPipeline()			   const { return m_VulkanPipeline; }
	private:
		void RT_invalidate();
		static void destroy(VkPipelineLayout pipelineLayout, VkPipeline vulkanPipeline);
		void createPipelineLayoutInfo();
		
		VkPipelineMultisampleStateCreateInfo			    createMultisampleInfo() const;
		VkPipelineRasterizationStateCreateInfo			    createRasterizationInfo() const;
		VkPipelineViewportStateCreateInfo				    createViewportStateInfo() const;
		VkPipelineVertexInputStateCreateInfo			    createVertexInputInfo(VkVertexInputBindingDescription& vertexInputBinding, std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs, bool instanced = false) const;
		VkPipelineDepthStencilStateCreateInfo			    createDepthStencilInfo() const;
		std::vector<VkPipelineColorBlendAttachmentState>    createColorBlendAttachments() const;
		VkPipelineColorBlendStateCreateInfo					createColorBlendInfo(std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments) const;
		VkPipelineDynamicStateCreateInfo					createDynamicStateInfo(std::vector<VkDynamicState>& dynamicStateEnables) const;
	private:
		PipelineSpecification m_Specification;
		VkPipelineLayout	  m_PipelineLayout;
		VkPipeline			  m_VulkanPipeline;
	};
}