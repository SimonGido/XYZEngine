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

	void OpenGLRenderTexture::BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const
	{
	}

	uint32_t OpenGLRenderTexture::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(
			m_RenderTarget->GetSpecification().Width, 
			m_RenderTarget->GetSpecification().Height
		);
	}

	std::pair<uint32_t, uint32_t> OpenGLRenderTexture::GetMipSize(uint32_t index) const
	{
		const auto& specs = m_RenderTarget->GetSpecification();
		uint32_t width = specs.Width;
		uint32_t height = specs.Height;
		while (index != 0)
		{
			width /= 2;
			height /= 2;
			index--;
		}

		return { width, height };
	}

}