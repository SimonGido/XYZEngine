#include "stdafx.h"
#include "VulkanRenderPass.h"


namespace XYZ {
	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
		:
		m_Specification(spec)
	{
	}
	VulkanRenderPass::~VulkanRenderPass()
	{
	}
}