#pragma once
#include "XYZ/Renderer/FrameBuffer.h"

namespace XYZ {

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecs& specs);
		virtual ~OpenGLFrameBuffer();

		virtual void Resize() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void Blit() const override;

		virtual void SetSpecification(const FrameBufferSpecs& specs) override;
	
		virtual void CreateColorAttachment(FrameBufferFormat format) override;
		virtual void CreateDepthAttachment() override;

		virtual ColorAttachment GetColorAttachment(uint32_t index) const override { return m_ColorAttachments[index]; }
		virtual DepthAttachment GetDetphAttachment(uint32_t index) const override { return m_DepthAttachments[index]; }

		virtual size_t GetNumberColorAttachments() const override { return m_ColorAttachments.size(); }
		virtual size_t GetNumberDepthAttachments() const override { return m_DepthAttachments.size(); }

		virtual const FrameBufferSpecs& GetSpecification() const override { return m_Specification; }
		virtual FrameBufferSpecs& GetSpecification() override { return m_Specification; }
	private:
		void setupColorAttachment(ColorAttachment& attachment, uint32_t index);
		void setupDepthAttachment(DepthAttachment& attachment);

		void setupMultiSample(ColorAttachment& attachment, DepthAttachment& depth);
	private:
		uint32_t m_RendererID = 0;
		FrameBufferSpecs m_Specification;

		std::vector<ColorAttachment> m_ColorAttachments;
		std::vector<DepthAttachment> m_DepthAttachments;
	};
}