#pragma once
#include "XYZ/Renderer/FrameBuffer.h"


namespace XYZ {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecs& specs);
		virtual ~OpenGLFramebuffer();

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot) const override;

		virtual void SetSpecification(const FramebufferSpecs& specs) override;


		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const override { return m_ColorAttachments[index]; }
		virtual const uint32_t GetDetphAttachmentRendererID() const override { return m_DepthAttachment; }

	
		virtual const FramebufferSpecs& GetSpecification() const override { return m_Specification; }
		virtual void ReadPixel(int32_t& pixel, uint32_t mx, uint32_t my, uint32_t attachmentIndex) const override;
		virtual void ClearColorAttachment(uint32_t colorAttachmentIndex, void* clearValue) const override;
	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecs m_Specification;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;

		std::vector<FramebufferTextureFormat> m_ColorAttachmentFormats;
		FramebufferTextureFormat m_DepthAttachmentFormat = FramebufferTextureFormat::None;
	};
}