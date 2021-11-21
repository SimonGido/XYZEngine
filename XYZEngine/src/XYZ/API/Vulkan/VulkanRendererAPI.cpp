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
#include "VulkanDescriptorAllocator.h"
#include "VulkanUniformBufferSet.h"
#include "VulkanMaterial.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	
	static VulkanDescriptorAllocator s_DescriptorAllocator;

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
		Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount)
	{
		Renderer::Submit([renderCommandBuffer, pipeline, uniformBufferSet, material, vertexBuffer, indexBuffer, indexCount]() mutable
		{
			XYZ_PROFILE_FUNC("VulkanRendererAPI::RenderGeometry");
			const VkDevice device    = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			const uint32_t frameIndex = VulkanContext::Get()->GetSwapChain().GetCurrentBufferIndex();
			Ref<VulkanRenderCommandBuffer> vulkanCommandBuffer = renderCommandBuffer;
			Ref<VulkanIndexBuffer>		   vulkanIndexBuffer = indexBuffer;
			Ref<VulkanShader>			   vulkanShader = pipeline->GetSpecification().Shader;
			Ref<VulkanPipeline>			   vulkanPipeline = pipeline;
			Ref<VulkanUniformBufferSet>	   vulkanUniformBufferSet = uniformBufferSet;
			Ref<VulkanMaterial>			   vulkanMaterial = material;
			const VkCommandBuffer		   commandBuffer = vulkanCommandBuffer->GetVulkanCommandBuffer(frameIndex);
			const VkPipelineLayout		   layout = vulkanPipeline->GetVulkanPipelineLayout();
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.As<VulkanPipeline>()->GetVulkanPipeline());


			const auto& uniformBufferDescriptors = vulkanUniformBufferSet->GetDescriptors(vulkanShader->GetHash());
			vulkanMaterial->RT_UpdateForRendering(uniformBufferDescriptors);
			
			const auto& materialDescriptors = vulkanMaterial->GetDescriptors(frameIndex);

			vkCmdBindDescriptorSets(
				commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 
				0, static_cast<uint32_t>(materialDescriptors.size()),
				materialDescriptors.data(), 0, nullptr
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

	VkDescriptorSet VulkanRendererAPI::RT_AllocateDescriptorSet(const VkDescriptorSetLayout& layout)
	{
		return s_DescriptorAllocator.RT_Allocate(layout);
	}

	void VulkanRendererAPI::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	void VulkanRendererAPI::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	{
		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	void VulkanRendererAPI::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);

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
