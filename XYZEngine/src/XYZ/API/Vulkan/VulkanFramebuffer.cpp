#include "stdafx.h"
#include "VulkanFramebuffer.h"

#include "XYZ/Core/Application.h"
#include "VulkanSwapChain.h"
#include "VulkanContext.h"

namespace XYZ {
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecs& specs)
		:
		m_Specification(specs),
		m_RenderPass(VK_NULL_HANDLE)
	{
		if (specs.Width == 0 || specs.Height == 0)
		{
			m_Specification.Width   = Application::Get().GetWindow().GetWidth();
			m_Specification.Height  = Application::Get().GetWindow().GetHeight();
		}
		Resize(m_Specification.Width, m_Specification.Height, true);
	}
	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		//if (!forceRecreate && (m_Width == width && m_Height == height))
		//	return;

		//m_Width = width * m_Specification.Scale;
		//m_Height = height * m_Specification.Scale;
		if (!m_Specification.SwapChainTarget)
		{
			//Invalidate();
		}
		else
		{
			Ref<VulkanContext> context = Application::Get().GetWindow().GetContext();
			VulkanSwapChain& swapChain = context->GetSwapChain();
			m_RenderPass = swapChain.GetRenderPass();

			//m_ClearValues.clear();
			//m_ClearValues.emplace_back().color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
	}
}