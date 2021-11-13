#include "stdafx.h"
#include "VulkanFramebuffer.h"

#include "XYZ/Core/Application.h"

namespace XYZ {
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecs& specs)
		:
		m_Specification(specs)
	{
		if (specs.Width == 0 || specs.Height == 0)
		{
			m_Specification.Width   = Application::Get().GetWindow().GetWidth();
			m_Specification.Height  = Application::Get().GetWindow().GetHeight();
		}
	}
}