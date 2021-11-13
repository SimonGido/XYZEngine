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

	private:
		void createPipelineLayoutInfo();
		
		VkPipelineMultisampleStateCreateInfo			    createMultisampleInfo() const;
		VkPipelineRasterizationStateCreateInfo			    createRasterizationInfo() const;
		VkPipelineViewportStateCreateInfo				    createViewportStateInfo() const;
		VkPipelineVertexInputStateCreateInfo			    createVertexInputInfo() const;
		VkPipelineDepthStencilStateCreateInfo			    createDepthStencilInfo() const;
		std::vector<VkPipelineColorBlendAttachmentState>    createColorBlendAttachments() const;
		VkPipelineColorBlendStateCreateInfo					createColorBlendInfo() const;
		VkPipelineDynamicStateCreateInfo					createDynamicStateInfo() const;
	private:
		PipelineSpecification m_Specification;
		VkPipelineLayout	  m_PipelineLayout;
		VkPipeline			  m_VulkanPipeline;
	};
}