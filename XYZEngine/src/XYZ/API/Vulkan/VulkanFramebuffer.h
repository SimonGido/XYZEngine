#pragma once
#include "XYZ/Renderer/Framebuffer.h"

namespace XYZ {
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecs& specs);

	private:
		FramebufferSpecs m_Specification;
	};
}