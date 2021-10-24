#include "OpenGLRenderPass.h"


namespace XYZ {
	OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
		:
		m_Specification(spec)
	{
	}

	OpenGLRenderPass::~OpenGLRenderPass()
	{
	}

	void OpenGLRenderPass::Release() const
	{
	}
}