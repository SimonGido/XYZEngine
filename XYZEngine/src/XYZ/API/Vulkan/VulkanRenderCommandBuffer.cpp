#include "stdafx.h"
#include "VulkanRenderCommandBuffer.h"

#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanImage.h"

namespace XYZ {
	VulkanPrimaryRenderCommandBuffer::VulkanPrimaryRenderCommandBuffer(uint32_t count, std::string debugName)
		:
		m_Name(std::move(debugName)),
		m_CommandBuffers({ VK_NULL_HANDLE }),
		m_CommandPool(VK_NULL_HANDLE),
		m_OwnedBySwapchain(false),
		m_TimestampQueryCount(0),
		m_NextTimestampQueryID(0),
		m_PipelineQueryCount(PipelineStatistics::Count())
	{
		auto device = VulkanContext::GetCurrentDevice();
		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_SecondaryVulkanCommandBuffers.resize(framesInFlight);


		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(device->GetVulkanDevice(), &cmdPoolInfo, nullptr, &m_CommandPool));
		
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = m_CommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (count == 0)
			count = framesInFlight;
		commandBufferAllocateInfo.commandBufferCount = count;
		m_CommandBuffers.resize(count);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device->GetVulkanDevice(), &commandBufferAllocateInfo, m_CommandBuffers.data()));


		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_WaitFences.resize(framesInFlight);
		for (auto& fence : m_WaitFences)
			VK_CHECK_RESULT(vkCreateFence(device->GetVulkanDevice(), &fenceCreateInfo, nullptr, &fence));
	
		
		createPipelineStatisticsQueries();
	}

	VulkanPrimaryRenderCommandBuffer::VulkanPrimaryRenderCommandBuffer(std::string debugName)
		:
		m_Name(std::move(debugName)),
		m_CommandPool(VK_NULL_HANDLE),
		m_OwnedBySwapchain(true),
		m_TimestampQueryCount(0),
		m_NextTimestampQueryID(0),
		m_PipelineQueryCount(PipelineStatistics::Count())
	{
		const VulkanSwapChain& swapChain = VulkanContext::GetSwapChain();
		auto device = swapChain.GetDevice();
		m_CommandBuffers.resize(swapChain.GetNumCommandsBuffers());
		m_SecondaryVulkanCommandBuffers.resize(swapChain.GetNumCommandsBuffers());

		for (size_t frame = 0; frame < swapChain.GetNumCommandsBuffers(); frame++)
			m_CommandBuffers[frame] = swapChain.GetCommandBuffer(frame);

		createPipelineStatisticsQueries();
	}
	VulkanPrimaryRenderCommandBuffer::~VulkanPrimaryRenderCommandBuffer()
	{	
		if (m_OwnedBySwapchain)
		{
			// Renderer is no longer valid, and all rendering is finished, it can be safely released here
			auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			for (const auto query : m_TimestampQueryPools)
				vkDestroyQueryPool(device, query, nullptr);
			for (const auto query : m_PipelineStatisticsQueryPools)
				vkDestroyQueryPool(device, query, nullptr);

			return;
		}

		destroyTimestampQueries();
		destroyPipelineStatisticsQueries();

		VkCommandPool commandPool = m_CommandPool;
		auto fences = m_WaitFences;
		Renderer::SubmitResource([commandPool, fences]()
		{
			auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			for (const auto fence : fences)
				vkDestroyFence(device, fence, nullptr);

			vkDestroyCommandPool(device, commandPool, nullptr);
		});
	}
	void VulkanPrimaryRenderCommandBuffer::Begin()
	{
		m_NextTimestampQueryID = 0;
		Ref<VulkanPrimaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_Begin();
		});
	}
	void VulkanPrimaryRenderCommandBuffer::End()
	{
		Ref<VulkanPrimaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]()mutable {
			instance->RT_End();
		});
	}
	void VulkanPrimaryRenderCommandBuffer::Submit()
	{
		if (m_OwnedBySwapchain)
			return;

		Ref<VulkanPrimaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]() mutable
		{
			auto device = VulkanContext::GetCurrentDevice();

			uint32_t frameIndex = Renderer::GetAPIContext()->GetCurrentFrame();

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer = instance->m_CommandBuffers[frameIndex];
			submitInfo.pCommandBuffers = &commandBuffer;

			VK_CHECK_RESULT(vkWaitForFences(device->GetVulkanDevice(), 1, &instance->m_WaitFences[frameIndex], VK_TRUE, UINT64_MAX));
			VK_CHECK_RESULT(vkResetFences(device->GetVulkanDevice(), 1, &instance->m_WaitFences[frameIndex]));
			VK_CHECK_RESULT(vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, instance->m_WaitFences[frameIndex]));
			
			instance->getTimestampQueryResults();
			instance->getPipelineQueryResults();
		});
	}

	void VulkanPrimaryRenderCommandBuffer::RT_Begin()
	{
		const uint32_t frameIndex = VulkanContext::Get()->GetCurrentFrame();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		const VkCommandBuffer commandBuffer = m_CommandBuffers[frameIndex];
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		XYZ_ASSERT(!m_Begin, "");
		m_Begin = true;
		if (!m_TimestampQueryPools.empty())
		{
			vkCmdResetQueryPool(commandBuffer, m_TimestampQueryPools[frameIndex], 0, m_TimestampQueryCount);
		}
		// Pipeline stats query		
		vkCmdResetQueryPool(commandBuffer, m_PipelineStatisticsQueryPools[frameIndex], 0, m_PipelineQueryCount);
		vkCmdBeginQuery(commandBuffer, m_PipelineStatisticsQueryPools[frameIndex], 0, 0);

	}

	void VulkanPrimaryRenderCommandBuffer::RT_End() 
	{
		const uint32_t frameIndex = VulkanContext::Get()->GetCurrentFrame();
     	const VkCommandBuffer commandBuffer = m_CommandBuffers[frameIndex];
		
		m_Begin = false;
		vkCmdEndQuery(commandBuffer, m_PipelineStatisticsQueryPools[frameIndex], 0);
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanPrimaryRenderCommandBuffer::CreateTimestampQueries(uint32_t count)
	{
		destroyTimestampQueries();
		createTimestampQueries(count);
	}

	float VulkanPrimaryRenderCommandBuffer::GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex) const
	{
		return m_ExecutionGPUTimes[frameIndex][queryIndex / 2];
	}
	uint32_t VulkanPrimaryRenderCommandBuffer::BeginTimestampQuery()
	{
		XYZ_ASSERT(m_NextTimestampQueryID < m_TimestampQueryCount, "Using more queries than created");
		uint32_t queryID = m_NextTimestampQueryID;
		m_NextTimestampQueryID += 2;
		Ref<VulkanPrimaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance, queryID]()
		{
			uint32_t frameIndex = Renderer::GetCurrentFrame();
			VkCommandBuffer commandBuffer = instance->m_CommandBuffers[frameIndex];
			vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_TimestampQueryPools[frameIndex], queryID);
		});
		return queryID;
	}
	void VulkanPrimaryRenderCommandBuffer::EndTimestampQuery(uint32_t queryID)
	{
		Ref<VulkanPrimaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance, queryID]()
		{
			uint32_t frameIndex = Renderer::GetCurrentFrame();
			VkCommandBuffer commandBuffer = instance->m_CommandBuffers[frameIndex];
			vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_TimestampQueryPools[frameIndex], queryID + 1);
		});
	}

	Ref<SecondaryRenderCommandBuffer> VulkanPrimaryRenderCommandBuffer::CreateSecondaryCommandBuffer()
	{
		Ref<VulkanSecondaryRenderCommandBuffer> secondaryBuffer = Ref<VulkanSecondaryRenderCommandBuffer>::Create(this);
		return secondaryBuffer;
	}

	VkResult VulkanPrimaryRenderCommandBuffer::GetFenceStatus(uint32_t index) const
	{
		const VulkanSwapChain& swapChain = VulkanContext::GetSwapChain();
		if (m_OwnedBySwapchain)
		{
			return swapChain.GetFenceStatus(index);
		}
		const auto device = swapChain.GetDevice()->GetVulkanDevice();
		return vkGetFenceStatus(device, m_WaitFences[index]);
	}

	void VulkanPrimaryRenderCommandBuffer::createTimestampQueries(uint32_t count)
	{
		m_TimestampQueryCount = count * 2;
		auto device = VulkanContext::GetCurrentDevice();
		uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		VkQueryPoolCreateInfo queryPoolCreateInfo = {};
		queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolCreateInfo.pNext = nullptr;

		

		queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryPoolCreateInfo.queryCount = m_TimestampQueryCount;
		m_TimestampQueryPools.resize(framesInFlight);
		for (auto& timestampQueryPool : m_TimestampQueryPools)
			VK_CHECK_RESULT(vkCreateQueryPool(device->GetVulkanDevice(), &queryPoolCreateInfo, nullptr, &timestampQueryPool));

		m_TimestampQueryResults.resize(framesInFlight);
		for (auto& timestampQueryResults : m_TimestampQueryResults)
			timestampQueryResults.resize(m_TimestampQueryCount);

		m_ExecutionGPUTimes.resize(framesInFlight);
		for (auto& executionGPUTimes : m_ExecutionGPUTimes)
			executionGPUTimes.resize(m_TimestampQueryCount / 2);
	}

	void VulkanPrimaryRenderCommandBuffer::createPipelineStatisticsQueries()
	{
		auto device = VulkanContext::GetCurrentDevice();
		uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;

		VkQueryPoolCreateInfo queryPoolCreateInfo = {};
		queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolCreateInfo.pNext = nullptr;

		
		queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
		queryPoolCreateInfo.queryCount = m_PipelineQueryCount;
		queryPoolCreateInfo.pipelineStatistics =
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

		m_PipelineStatisticsQueryPools.resize(framesInFlight);
		for (auto& pipelineStatisticsQueryPools : m_PipelineStatisticsQueryPools)
			VK_CHECK_RESULT(vkCreateQueryPool(device->GetVulkanDevice(), &queryPoolCreateInfo, nullptr, &pipelineStatisticsQueryPools));

		m_PipelineStatisticsQueryResults.resize(framesInFlight);
	}

	void VulkanPrimaryRenderCommandBuffer::destroyTimestampQueries()
	{
		if (m_TimestampQueryPools.empty())
			return;

		auto timestampQueryPools = m_TimestampQueryPools;
		Renderer::SubmitResource([timestampQueryPools]()
		{
			auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			for (const auto query : timestampQueryPools)
				vkDestroyQueryPool(device, query, nullptr);
		});
	}

	void VulkanPrimaryRenderCommandBuffer::destroyPipelineStatisticsQueries()
	{
		if (m_PipelineStatisticsQueryPools.empty())
			return;

		auto pipelineStatisticsQueryPools = m_PipelineStatisticsQueryPools;
		Renderer::SubmitResource([pipelineStatisticsQueryPools]()
		{
			auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			for (const auto query : pipelineStatisticsQueryPools)
				vkDestroyQueryPool(device, query, nullptr);
		});
	}

	void VulkanPrimaryRenderCommandBuffer::getTimestampQueryResults()
	{
		auto device = VulkanContext::GetCurrentDevice();
		const uint32_t frameIndex = Renderer::GetCurrentFrame();


		vkGetQueryPoolResults(device->GetVulkanDevice(), m_TimestampQueryPools[frameIndex], 0, m_TimestampQueryCount,
			m_TimestampQueryCount * sizeof(uint64_t), m_TimestampQueryResults[frameIndex].data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

		for (uint32_t i = 0; i < m_TimestampQueryCount; i += 2)
		{
			uint64_t startTime = m_TimestampQueryResults[frameIndex][i];
			uint64_t endTime = m_TimestampQueryResults[frameIndex][i + 1];
			float nsTime = endTime > startTime ? (endTime - startTime) * device->GetPhysicalDevice()->GetLimits().timestampPeriod : 0.0f;
			m_ExecutionGPUTimes[frameIndex][i / 2] = nsTime * 0.000001f; // Time in ms
		}
	}
	void VulkanPrimaryRenderCommandBuffer::getPipelineQueryResults()
	{
		auto device = VulkanContext::GetCurrentDevice();
		const uint32_t frameIndex = Renderer::GetCurrentFrame();

		vkGetQueryPoolResults(device->GetVulkanDevice(), m_PipelineStatisticsQueryPools[frameIndex], 0, 1,
			sizeof(PipelineStatistics), &m_PipelineStatisticsQueryResults[frameIndex], sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
	}
	VulkanSecondaryRenderCommandBuffer::VulkanSecondaryRenderCommandBuffer(Ref<RenderCommandBuffer> primaryCommandBuffer)
		:
		m_PrimaryRenderCommandBuffer(primaryCommandBuffer)
	{
		const uint32_t count = Renderer::GetConfiguration().FramesInFlight;
		
		m_CommandBuffers.resize(static_cast<size_t>(count));
		auto device = VulkanContext::GetCurrentDevice();
		for (uint32_t i = 0; i < count; ++i)
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = m_PrimaryRenderCommandBuffer->m_CommandPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			cmdBufAllocateInfo.commandBufferCount = 1;

			VK_CHECK_RESULT(vkAllocateCommandBuffers(device->GetVulkanDevice(), &cmdBufAllocateInfo, &m_CommandBuffers[i]));
		
			m_PrimaryRenderCommandBuffer->m_SecondaryVulkanCommandBuffers[i].push_back(m_CommandBuffers[i]);
		}
	}
	VulkanSecondaryRenderCommandBuffer::~VulkanSecondaryRenderCommandBuffer()
	{
	}
	void VulkanSecondaryRenderCommandBuffer::Begin(Ref<Framebuffer> framebuffer, bool clear)
	{
		Ref<VulkanSecondaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance, framebuffer, clear]() mutable {
			instance->RT_Begin(framebuffer, clear);
		});
	}
	void VulkanSecondaryRenderCommandBuffer::End()
	{
		Ref<VulkanSecondaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_End();
		});
	}

	void VulkanSecondaryRenderCommandBuffer::RT_Begin(Ref<Framebuffer> framebuffer, bool clear)
	{
		const uint32_t frameIndex = VulkanContext::Get()->GetCurrentFrame();
		Ref<VulkanContext> vulkanContext = Renderer::GetAPIContext();
		Ref<VulkanFramebuffer> vulkanFrameBuffer = framebuffer.As<VulkanFramebuffer>();

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = vulkanFrameBuffer->GetRenderPass();
		inheritanceInfo.framebuffer = vulkanFrameBuffer->GetFramebuffer();
		inheritanceInfo.pipelineStatistics = 
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		const VkCommandBuffer commandBuffer = m_CommandBuffers[frameIndex];
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		uint32_t width = framebuffer->GetSpecification().Width;
		uint32_t height = framebuffer->GetSpecification().Height;

		VkViewport viewport = { 0.0f, static_cast<float>(height) };
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = width;
		viewport.height = -static_cast<float>(height);


		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent.width = width;
		scissor.extent.height = height;

		if (framebuffer->GetSpecification().SwapChainTarget)
		{
			const VulkanSwapChain& swapChain = vulkanContext->GetSwapChain();
			const auto extent = swapChain.GetExtent();
			viewport = { 0.0f, static_cast<float>(extent.height) };
			viewport.width = static_cast<float>(extent.width);
			viewport.height = -static_cast<float>(extent.height);
			scissor.extent = extent;
		}
		if (clear)
			clearFramebuffer(framebuffer.As<VulkanFramebuffer>(), commandBuffer);

		// Update dynamic viewport state
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		// Update dynamic scissor state				
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void VulkanSecondaryRenderCommandBuffer::RT_End()
	{
		const uint32_t frameIndex = VulkanContext::Get()->GetCurrentFrame();
		const VkCommandBuffer commandBuffer = m_CommandBuffers[frameIndex];

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}
	void VulkanSecondaryRenderCommandBuffer::Submit()
	{
		Ref<VulkanSecondaryRenderCommandBuffer> instance = this;
		Renderer::Submit([instance]() mutable {

			const uint32_t frame = Renderer::GetCurrentFrame();
			const VkCommandBuffer commandBuffer = (const VkCommandBuffer)instance->m_PrimaryRenderCommandBuffer->CommandBufferHandle(frame);
			vkCmdExecuteCommands(commandBuffer, 1u, &instance->m_CommandBuffers[frame]);
		});
	}
	void VulkanSecondaryRenderCommandBuffer::clearFramebuffer(Ref<VulkanFramebuffer> framebuffer, const VkCommandBuffer commandBuffer)
	{
		const uint32_t colorAttachmentCount = framebuffer->GetNumColorAttachments();
		const auto& clearValues = framebuffer->GetVulkanClearValues();


		const uint32_t width = framebuffer->GetSpecification().Width;
		const uint32_t height = framebuffer->GetSpecification().Height;
		std::vector<VkClearAttachment> attachments(colorAttachmentCount);
		std::vector<VkClearRect> clearRects(colorAttachmentCount);
		for (uint32_t i = 0; i < colorAttachmentCount; i++)
		{
			attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachments[i].colorAttachment = i;
			attachments[i].clearValue = clearValues[i];

			clearRects[i].rect.offset = { 0, 0 };
			clearRects[i].rect.extent = { width, height };
			clearRects[i].baseArrayLayer = 0;
			clearRects[i].layerCount = 1;
		}

		if (framebuffer->HasDepthAttachment())
		{
			Ref<VulkanImage2D> depthImage = framebuffer->GetDepthImage();
			attachments.push_back({});
			clearRects.push_back({});
			attachments[colorAttachmentCount].aspectMask = depthImage->GetImageViewAspectFlags();
			attachments[colorAttachmentCount].clearValue = clearValues[colorAttachmentCount];
			clearRects[colorAttachmentCount].rect.offset = { 0, 0 };
			clearRects[colorAttachmentCount].rect.extent = { width, height };
			clearRects[colorAttachmentCount].baseArrayLayer = 0;
			clearRects[colorAttachmentCount].layerCount = 1;
		}

		vkCmdClearAttachments(commandBuffer, attachments.size(), attachments.data(), clearRects.size(), clearRects.data());
	}
}