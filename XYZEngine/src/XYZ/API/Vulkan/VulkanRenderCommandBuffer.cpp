#include "stdafx.h"
#include "VulkanRenderCommandBuffer.h"

#include "VulkanContext.h"
#include "VulkanSwapChain.h"

namespace XYZ {
	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(uint32_t count, std::string debugName)
		:
		m_Name(std::move(debugName)),
		m_CommandBuffers(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_SwapChain(nullptr),
		m_OwnedBySwapchain(false)
	{
		//TODO
	}

	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(std::string debugName, VulkanSwapChain& swapChain)
		:
		m_Name(std::move(debugName)),
		m_CommandPool(VK_NULL_HANDLE),
		m_SwapChain(&swapChain),
		m_OwnedBySwapchain(true)
	{
		auto device = swapChain.GetDevice();
		m_CommandBuffers.resize(swapChain.GetNumCommandsBuffers());

		for (size_t frame = 0; frame < swapChain.GetNumCommandsBuffers(); frame++)
			m_CommandBuffers[frame] = swapChain.GetCommandBuffer(frame);

	}
	VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
	{
		if (m_OwnedBySwapchain)
			return;

		VkCommandPool commandPool = m_CommandPool;
		Renderer::Submit([commandPool]()
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkDestroyCommandPool(device->GetVulkanDevice(), commandPool, nullptr);
		});
	}
	void VulkanRenderCommandBuffer::Begin()
	{
		Ref<VulkanRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]() mutable {

			uint32_t frameIndex = instance->m_SwapChain->GetCurrentBufferIndex();
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			VkCommandBuffer commandBuffer = instance->m_CommandBuffers[frameIndex];
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

			//// Timestamp query
			//vkCmdResetQueryPool(commandBuffer, instance->m_TimestampQueryPools[frameIndex], 0, instance->m_TimestampQueryCount);
			//vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_TimestampQueryPools[frameIndex], 0);
			//
			//// Pipeline stats query
			//vkCmdResetQueryPool(commandBuffer, instance->m_PipelineStatisticsQueryPools[frameIndex], 0, instance->m_PipelineQueryCount);
			//vkCmdBeginQuery(commandBuffer, instance->m_PipelineStatisticsQueryPools[frameIndex], 0, 0);
		});
	}
	void VulkanRenderCommandBuffer::End()
	{
		Ref<VulkanRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]()
		{
			uint32_t frameIndex = instance->m_SwapChain->GetCurrentBufferIndex();
			//uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
			VkCommandBuffer commandBuffer = instance->m_CommandBuffers[frameIndex];
			//vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_TimestampQueryPools[frameIndex], 1);
			//vkCmdEndQuery(commandBuffer, instance->m_PipelineStatisticsQueryPools[frameIndex], 0);
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		});

	}
	void VulkanRenderCommandBuffer::Submit()
	{
	}
	float VulkanRenderCommandBuffer::GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex) const
	{
		return 0.0f;
	}
	uint64_t VulkanRenderCommandBuffer::BeginTimestampQuery()
	{
		return uint64_t();
	}
	void VulkanRenderCommandBuffer::EndTimestampQuery(uint64_t queryID)
	{
	}

}