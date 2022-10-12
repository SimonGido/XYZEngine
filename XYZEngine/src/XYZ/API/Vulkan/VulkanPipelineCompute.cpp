#include "stdafx.h"
#include "VulkanPipelineCompute.h"


namespace XYZ {
	static VkFence s_ComputeFence = nullptr;

	VulkanPipelineCompute::VulkanPipelineCompute(Ref<Shader> computeShader)
		: m_Shader(computeShader.As<VulkanShader>())
	{
		Ref<VulkanPipelineCompute> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_createPipeline();
		});

		Renderer::RegisterShaderDependency(computeShader, this);
	}

	VulkanPipelineCompute::~VulkanPipelineCompute()
	{
		destroy();
	}

	void VulkanPipelineCompute::CreatePipeline()
	{
		Renderer::Submit([instance = Ref(this)]() mutable
		{
			instance->RT_createPipeline();
		});
	}

	void VulkanPipelineCompute::Invalidate()
	{
		destroy();
		CreatePipeline();
	}

	void VulkanPipelineCompute::Begin(Ref<RenderCommandBuffer> renderCommandBuffer)
	{
		XYZ_ASSERT(!m_ActiveComputeCommandBuffer, "");


		if (renderCommandBuffer.Raw())
		{
			const uint32_t frameIndex = Renderer::GetCurrentFrame();
			m_ActiveComputeCommandBuffer = (const VkCommandBuffer)renderCommandBuffer->CommandBufferHandle(frameIndex);
			m_UsingGraphicsQueue = true;
		}
		else
		{
			m_ActiveComputeCommandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true, true);
			m_UsingGraphicsQueue = false;
		}
		vkCmdBindPipeline(m_ActiveComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);
	}

	void VulkanPipelineCompute::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		XYZ_ASSERT(m_ActiveComputeCommandBuffer, "");

		vkCmdDispatch(m_ActiveComputeCommandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanPipelineCompute::End()
	{
		XYZ_ASSERT(m_ActiveComputeCommandBuffer, "");

		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		if (!m_UsingGraphicsQueue)
		{
			VkQueue computeQueue = VulkanContext::GetCurrentDevice()->GetComputeQueue();

			vkEndCommandBuffer(m_ActiveComputeCommandBuffer);

			if (!s_ComputeFence)
			{
				VkFenceCreateInfo fenceCreateInfo{};
				fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &s_ComputeFence));
			}
			vkWaitForFences(device, 1, &s_ComputeFence, VK_TRUE, UINT64_MAX);
			vkResetFences(device, 1, &s_ComputeFence);

			VkSubmitInfo computeSubmitInfo{};
			computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			computeSubmitInfo.commandBufferCount = 1;
			computeSubmitInfo.pCommandBuffers = &m_ActiveComputeCommandBuffer;
			VK_CHECK_RESULT(vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, s_ComputeFence));

			// Wait for execution of compute shader to complete
			// Currently this is here for "safety"
			{
				vkWaitForFences(device, 1, &s_ComputeFence, VK_TRUE, UINT64_MAX);
			}
		}
		m_ActiveComputeCommandBuffer = nullptr;
	}

	void VulkanPipelineCompute::SetPushConstants(const void* data, uint32_t size)
	{
		vkCmdPushConstants(m_ActiveComputeCommandBuffer, m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
	}

	void VulkanPipelineCompute::destroy()
	{
		VkPipelineLayout pipelineLayout = m_ComputePipelineLayout;
		VkPipeline		 vulkanPipeline = m_ComputePipeline;
		VkPipelineCache  cache = m_PipelineCache;
		Renderer::SubmitResource([pipelineLayout, vulkanPipeline, cache]() {
			if (pipelineLayout != VK_NULL_HANDLE && vulkanPipeline != VK_NULL_HANDLE)
			{
				const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
				VK_CHECK_RESULT(vkDeviceWaitIdle(device));
				vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
				vkDestroyPipeline(device, vulkanPipeline, nullptr);
				vkDestroyPipelineCache(device, cache, nullptr);
			}
			});
	}

	void VulkanPipelineCompute::RT_createPipeline()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		auto descriptorSetLayouts = m_Shader->GetAllDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

		const auto& pushConstantRanges = m_Shader->GetPushConstantRanges();
		std::vector<VkPushConstantRange> vulkanPushConstantRanges(pushConstantRanges.size());
		if (pushConstantRanges.size())
		{
			for (uint32_t i = 0; i < pushConstantRanges.size(); i++)
			{
				const auto& pushConstantRange = pushConstantRanges[i];
				auto& vulkanPushConstantRange = vulkanPushConstantRanges[i];

				vulkanPushConstantRange.stageFlags = pushConstantRange.ShaderStage;
				vulkanPushConstantRange.offset = pushConstantRange.Offset;
				vulkanPushConstantRange.size = pushConstantRange.Size;
			}

			pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)vulkanPushConstantRanges.size();
			pipelineLayoutCreateInfo.pPushConstantRanges = vulkanPushConstantRanges.data();
		}

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_ComputePipelineLayout));

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_ComputePipelineLayout;
		computePipelineCreateInfo.flags = 0;
		const auto& shaderStages = m_Shader->GetPipelineShaderStageCreateInfos();
		computePipelineCreateInfo.stage = shaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
		VK_CHECK_RESULT(vkCreateComputePipelines(device, m_PipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_ComputePipeline));
	}
}