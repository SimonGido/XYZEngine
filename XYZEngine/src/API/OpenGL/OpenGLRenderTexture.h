#pragma once
#include "XYZ/Renderer/RenderTexture.h"


namespace XYZ {
	class OpenGLRenderTexture : public RenderTexture
	{
	public:
		OpenGLRenderTexture(const Ref<FrameBuffer>& renderTarget);
		virtual ~OpenGLRenderTexture();
		virtual void Bind(uint32_t slot) const override;
		
		virtual uint32_t GetWidth() const override { return m_RenderTarget->GetSpecification().Width; };
		virtual uint32_t GetHeight() const override { return m_RenderTarget->GetSpecification().Height; };
		virtual uint32_t GetChannels() const override { return 4; }; // TODO: return real number of channels

		virtual uint32_t GetRendererID() const override { return m_RenderTarget->GetColorAttachmentRendererID(0); };
		virtual Ref<FrameBuffer> GetRenderTarget() override { return m_RenderTarget; }
	private:
		Ref<FrameBuffer> m_RenderTarget;
	};
}