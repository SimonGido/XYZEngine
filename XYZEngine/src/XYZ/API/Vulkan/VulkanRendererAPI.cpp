#include "stdafx.h"
#include "VulkanRendererAPI.h"

#include "VulkanRenderCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanUniformBuffer.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	class DescriptorAllocator
	{
	public:
		static constexpr uint32_t sc_NumPoolSizes = 11;
		static constexpr uint32_t sc_MaxSets = 100000;

		void Init()
		{		
			Renderer::Submit([this]() {

				VkDescriptorPoolSize pool_sizes[sc_NumPoolSizes] =
				{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
				};
				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = 0;
				pool_info.maxSets = sc_MaxSets;
				pool_info.poolSizeCount = sc_NumPoolSizes;
				pool_info.pPoolSizes = pool_sizes;
				VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

				const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
				m_DescriptorPools.resize(framesInFlight);
				m_AllocationCount.resize(framesInFlight);
				for (uint32_t i = 0; i < framesInFlight; i++)
				{
					VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPools[i]));
					m_AllocationCount[i] = 0;
				}
			});
		}

		void Shutdown()
		{
			Renderer::SubmitResourceFree([this]() {
				VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

				for (const auto descr : m_DescriptorPools)
					vkDestroyDescriptorPool(device, descr, nullptr);

				m_DescriptorPools.clear();
				m_AllocationCount.clear();
			});
		}

		VkDescriptorSet RT_Allocate(VkDescriptorSetAllocateInfo& allocInfo)
		{
			uint32_t bufferIndex = Renderer::GetAPIContext()->GetCurrentFrame();
			allocInfo.descriptorPool = m_DescriptorPools[bufferIndex];
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			VkDescriptorSet result;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &result));
			m_AllocationCount[bufferIndex] += allocInfo.descriptorSetCount;
			return result;
		}

	private:
		std::vector<VkDescriptorPool> m_DescriptorPools;
		std::vector<uint32_t>		  m_AllocationCount;
	};

	static DescriptorAllocator s_DescriptorAllocator;

	void VulkanRendererAPI::Init()
	{
		s_DescriptorAllocator.Init();
	}

	void VulkanRendererAPI::Shutdown()
	{
		s_DescriptorAllocator.Shutdown();
	}

	void VulkanRendererAPI::BeginFrame()
	{
		Renderer::Submit([]()
		{
			Ref<VulkanContext> context = Renderer::GetAPIContext();
			VulkanSwapChain& swapChain = context->GetSwapChain();
			const VkCommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();
			VkCommandBufferBeginInfo cmdBufInfo = {};
			cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			cmdBufInfo.pNext = nullptr;
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
		});
	}

	void VulkanRendererAPI::EndFrame()
	{
		Renderer::Submit([]()
		{
			Ref<VulkanContext> context = Renderer::GetAPIContext();
			VulkanSwapChain& swapChain = context->GetSwapChain();
			const VkCommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();
			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		});
	}

	void VulkanRendererAPI::BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer,
		const Ref<RenderPass>& renderPass, bool explicitClear)
	{
		Renderer::Submit([renderCommandBuffer, renderPass, explicitClear]()
		{
			XYZ_PROFILE_FUNC("VulkanRendererAPI::BeginRenderPass");
			Ref<VulkanFramebuffer> framebuffer = renderPass->GetSpecification().TargetFramebuffer;
			Ref<VulkanContext> vulkanContext = Renderer::GetAPIContext();
			const uint32_t frameIndex = vulkanContext->GetSwapChain().GetCurrentBufferIndex();
			uint32_t width = framebuffer->GetSpecification().Width;
			uint32_t height = framebuffer->GetSpecification().Height;

			VkViewport viewport = {};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = width;
			renderPassBeginInfo.renderArea.extent.height = height;
			if (framebuffer->GetSpecification().SwapChainTarget)
			{
				const VulkanSwapChain& swapChain = vulkanContext->GetSwapChain();
				width = swapChain.GetWidth();
				height = swapChain.GetHeight();
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.pNext = nullptr;
				renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
				renderPassBeginInfo.renderArea.offset = { 0, 0 };
				renderPassBeginInfo.renderArea.extent = swapChain.GetExtent();
				renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

				viewport = { 0.0f, 0.0f };
				viewport.width = static_cast<float>(width);
				viewport.height = static_cast<float>(height);

				scissor.extent = swapChain.GetExtent();
			}


			const VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearColor;
			const VkCommandBuffer commandBuffer = renderCommandBuffer.As<VulkanRenderCommandBuffer>()->GetVulkanCommandBuffer(frameIndex);
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			if (explicitClear)
				clearFramebuffer(framebuffer, commandBuffer);
			// Update dynamic viewport state
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			// Update dynamic scissor state				
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		});
	}

	void VulkanRendererAPI::EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer)
	{
		Renderer::Submit([renderCommandBuffer]()
		{
			XYZ_PROFILE_FUNC("VulkanRendererAPI::EndRenderPass");
			const uint32_t frameIndex = VulkanContext::Get()->GetSwapChain().GetCurrentBufferIndex();
			const VkCommandBuffer commandBuffer = renderCommandBuffer.As<VulkanRenderCommandBuffer>()->GetVulkanCommandBuffer(frameIndex);
			vkCmdEndRenderPass(commandBuffer);
		});
	}

	void VulkanRendererAPI::RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet,
		Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount)
	{
		Renderer::Submit([renderCommandBuffer, pipeline, uniformBufferSet, vertexBuffer, indexBuffer, indexCount]() mutable
		{
			XYZ_PROFILE_FUNC("VulkanRendererAPI::RenderGeometry");
			const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			const uint32_t frameIndex = VulkanContext::Get()->GetSwapChain().GetCurrentBufferIndex();
			Ref<VulkanRenderCommandBuffer> vulkanCommandBuffer = renderCommandBuffer;
			Ref<VulkanIndexBuffer>		   vulkanIndexBuffer = indexBuffer;
			Ref<VulkanShader>			   vulkanShader = pipeline->GetSpecification().Shader;
			Ref<VulkanPipeline>			   vulkanPipeline = pipeline;
			const VkCommandBuffer		   commandBuffer = vulkanCommandBuffer->GetVulkanCommandBuffer(frameIndex);
			const VkPipelineLayout		   layout = vulkanPipeline->GetVulkanPipelineLayout();
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.As<VulkanPipeline>()->GetVulkanPipeline());


			const auto& uniformBufferWriteDescription = vulkanShader->GetUniformBufferWriteDescriptions();
			const auto& frameUniformBufferDescr = uniformBufferWriteDescription[frameIndex];

			std::vector<VkWriteDescriptorSet> writeDescriptors;
			const std::vector<VkDescriptorSet>& descriptorSets = frameUniformBufferDescr.DescriptorSets;
			for (uint32_t set = 0; set < frameUniformBufferDescr.DescriptorSets.size(); ++set)
			{
				uint32_t binding = frameUniformBufferDescr.Bindings[set];
				VkWriteDescriptorSet writeDescriptor = frameUniformBufferDescr.WriteDescriptorSets[set];
				Ref<VulkanUniformBuffer> uniformBuffer = uniformBufferSet->Get(binding, set, frameIndex);
				writeDescriptor.pBufferInfo = &uniformBuffer->GetDescriptorBufferInfo();
				writeDescriptors.push_back(writeDescriptor);
			}

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
			vkCmdBindDescriptorSets(
				commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 
				0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr
			);

			///////////////////////////////		
			const VkBuffer vertexBuffers[] = { vertexBuffer.As<VulkanVertexBuffer>()->GetVulkanBuffer() };
			const VkDeviceSize offsets[] = { 0 };
			if (indexCount == 0)
				indexCount = indexBuffer->GetCount();
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetVulkanBuffer(), 0, vulkanIndexBuffer->GetVulkanIndexType());
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
			
		});
	}

	VkDescriptorSet VulkanRendererAPI::RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		return s_DescriptorAllocator.RT_Allocate(allocInfo);
	}


	void VulkanRendererAPI::clearFramebuffer(Ref<VulkanFramebuffer> framebuffer, VkCommandBuffer commandBuffer)
	{
		const uint32_t colorAttachmentCount = framebuffer->GetNumColorAttachments();
		const uint32_t totalAttachmentCount = colorAttachmentCount; // + depth
		const auto& clearValues = framebuffer->GetVulkanClearValues();
		XYZ_ASSERT(clearValues.size() == totalAttachmentCount, "");


		const uint32_t width = framebuffer->GetSpecification().Width;
		const uint32_t height = framebuffer->GetSpecification().Height;
		std::vector<VkClearAttachment> attachments(totalAttachmentCount);
		std::vector<VkClearRect> clearRects(totalAttachmentCount);
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
			attachments[colorAttachmentCount].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			attachments[colorAttachmentCount].clearValue = clearValues[colorAttachmentCount];
			clearRects[colorAttachmentCount].rect.offset = { 0, 0 };
			clearRects[colorAttachmentCount].rect.extent = { width, height };
			clearRects[colorAttachmentCount].baseArrayLayer = 0;
			clearRects[colorAttachmentCount].layerCount = 1;
		}

		vkCmdClearAttachments(commandBuffer, totalAttachmentCount, attachments.data(), totalAttachmentCount, clearRects.data());
	}
}
