#pragma once
#include "XYZ/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace XYZ {
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecs& specs);

		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		virtual const FramebufferSpecs& GetSpecification() const override { return m_Specification; }
	private:
		FramebufferSpecs m_Specification;
		VkRenderPass	 m_RenderPass;
	};
}