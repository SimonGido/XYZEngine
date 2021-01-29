#include "stdafx.h"
#include "OpenGLRenderTexture.h"


namespace XYZ {

	OpenGLRenderTexture::OpenGLRenderTexture(const Ref<Framebuffer>& renderTarget)
		: m_RenderTarget(renderTarget)
	{
		XYZ_ASSERT(!m_RenderTarget->GetSpecification().Attachments.Attachments.empty(), "Render target has no attachments");
	}

	OpenGLRenderTexture::~OpenGLRenderTexture()
	{
	}

	void OpenGLRenderTexture::Bind(uint32_t slot) const
	{		
		m_RenderTarget->BindTexture(0, slot);
	}

}