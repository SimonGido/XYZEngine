#pragma once
#include "VulkanFramebuffer.h"
#include "XYZ/Renderer/RendererAPI.h"

namespace XYZ {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<RenderPass> renderPass, bool explicitClear = false) override;
		virtual void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer) override;
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform, uint32_t indexCount = 0) override;
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount = 0) override;
		virtual void BindPipeline(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material) override;


		static VkDescriptorSet RT_AllocateDescriptorSet(const VkDescriptorSetLayout& layout);
	
		static void InsertImageMemoryBarrier(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkAccessFlags srcAccessMask,
				VkAccessFlags dstAccessMask,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkPipelineStageFlags srcStageMask,
				VkPipelineStageFlags dstStageMask,
				VkImageSubresourceRange subresourceRange
		);

		static void SetImageLayout(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkImageSubresourceRange subresourceRange,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		static void SetImageLayout(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkImageAspectFlags aspectMask,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);
	private:
		static void clearFramebuffer(Ref<VulkanFramebuffer> framebuffer, VkCommandBuffer commandBuffer);
	};
}