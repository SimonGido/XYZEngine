#pragma once
#include "XYZ/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace XYZ {
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecs& specs);

		virtual void					Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
		virtual uint32_t				GetColorAttachmentCount()		   const { return m_Specification.SwapChainTarget ? 1 : 0; }
		virtual const FramebufferSpecs& GetSpecification() const override { return m_Specification; }

		VkRenderPass					 GetRenderPass() const { return m_RenderPass; }
		const std::vector<VkClearValue>& GetVulkanClearValues() const { return m_ClearValues; }
		bool							 HasDepthAttachment() const { return false; }

		void Invalidate();
		void RT_Invalidate() const;
	private:
		FramebufferSpecs m_Specification;
		VkRenderPass	 m_RenderPass;
		VkFramebuffer	 m_Framebuffer;
		std::vector<VkClearValue> m_ClearValues;

		friend class VulkanSwapChain;
	};
}