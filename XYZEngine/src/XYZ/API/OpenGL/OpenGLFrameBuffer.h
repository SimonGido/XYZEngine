#pragma once
#include "XYZ/Renderer/FrameBuffer.h"


namespace XYZ {

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecs& specs);
		virtual ~OpenGLFrameBuffer();

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot) const override;

		virtual void SetSpecification(const FrameBufferSpecs& specs) override;


		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const override { return m_ColorAttachments[index]; }
		virtual const uint32_t GetDetphAttachmentRendererID() const override { return m_DepthAttachment; }

	
		virtual const FrameBufferSpecs& GetSpecification() const override { return m_Specification; }
	
	private:
		uint32_t m_RendererID = 0;
		FrameBufferSpecs m_Specification;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;

		std::vector<FrameBufferTextureFormat> m_ColorAttachmentFormats;
		FrameBufferTextureFormat m_DepthAttachmentFormat = FrameBufferTextureFormat::None;
	};
}