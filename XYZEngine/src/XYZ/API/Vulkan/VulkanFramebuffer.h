#pragma once
#include "XYZ/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace XYZ {
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecs& specs);

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
		
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		virtual const FramebufferSpecs& GetSpecification() const override { return m_Specification; }


		void Invalidate();
		void RT_Invalidate();
	private:
		FramebufferSpecs m_Specification;
		VkRenderPass	 m_RenderPass;
		VkFramebuffer	 m_Framebuffer;
		std::vector<VkClearValue> m_ClearValues;
		
		friend class VulkanSwapChain;
	};
}