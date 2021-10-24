#pragma once
#include "XYZ/Renderer/RenderTexture.h"


namespace XYZ {
	class OpenGLRenderTexture : public RenderTexture
	{
	public:
		OpenGLRenderTexture(const Ref<Framebuffer>& renderTarget);
		virtual ~OpenGLRenderTexture();
		virtual void Release() const override;

		virtual void Bind(uint32_t slot) const override;
		virtual void BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const override;

		virtual uint32_t GetWidth() const override { return m_RenderTarget->GetSpecification().Width; };
		virtual uint32_t GetHeight() const override { return m_RenderTarget->GetSpecification().Height; };
		virtual uint32_t GetChannelSize() const override { return 4; }; // TODO: return real number of channels
		virtual uint32_t GetMipLevelCount() const override;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t index) const override;
		virtual uint32_t GetRendererID() const override { return m_RenderTarget->GetColorAttachmentRendererID(0); };
		virtual Ref<Framebuffer> GetRenderTarget() override { return m_RenderTarget; }
	private:
		Ref<Framebuffer> m_RenderTarget;
	};
}