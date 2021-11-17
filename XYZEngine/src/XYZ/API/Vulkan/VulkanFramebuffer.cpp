#include "stdafx.h"
#include "VulkanFramebuffer.h"

#include "XYZ/Core/Application.h"
#include "VulkanSwapChain.h"
#include "VulkanContext.h"

namespace XYZ {
	namespace Utils
	{
		VkClearColorValue GlmColorToVkColor(const glm::vec4& color)
		{
			return { color.r, color.g , color.b, color.a };
		}
	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecs& specs)
		:
		m_Specification(specs),
		m_RenderPass(VK_NULL_HANDLE),
		m_Framebuffer(VK_NULL_HANDLE)
	{
		if (specs.Width == 0 || specs.Height == 0)
		{
			m_Specification.Width = Application::Get().GetWindow().GetWidth();
			m_Specification.Height = Application::Get().GetWindow().GetHeight();
		}
		Resize(m_Specification.Width, m_Specification.Height, true);
	}
	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		if (!forceRecreate && (m_Specification.Width == width && m_Specification.Height == height))
			return;

		if (!m_Specification.SwapChainTarget)
		{
			Invalidate();
		}
		else
		{
			Ref<VulkanContext> context = Renderer::GetAPIContext();
			const VulkanSwapChain& swapChain = context->GetSwapChain();
			m_RenderPass = swapChain.GetVulkanRenderPass();

			m_ClearValues.clear();
			m_ClearValues.emplace_back().color = Utils::GlmColorToVkColor(m_Specification.ClearColor);
		}
	}

	void VulkanFramebuffer::Invalidate()
	{
		Ref< VulkanFramebuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_Invalidate();
		});
	}

	void VulkanFramebuffer::RT_Invalidate() const
	{
		auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		if (m_Framebuffer)
		{
			VkFramebuffer framebuffer = m_Framebuffer;
			Renderer::SubmitResourceFree([framebuffer]() {
				const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			});
		}
	}
}
