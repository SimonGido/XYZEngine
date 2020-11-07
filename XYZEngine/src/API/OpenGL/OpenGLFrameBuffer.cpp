#include "stdafx.h"
#include "OpenGLFrameBuffer.h"

#include "XYZ/Renderer/Renderer.h"

#include <GL/glew.h>

namespace XYZ {
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs)
		: m_Specification(specs)
	{
		Resize();
	}
	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		Renderer::Submit([this]() {
			glDeleteFramebuffers(1, &m_RendererID);
			for (auto it : m_ColorAttachments)
				glDeleteTextures(1, &it.RendererID);
			for (auto it : m_DepthAttachments)
				glDeleteTextures(1, &it.RendererID);
			});
	}
	void OpenGLFrameBuffer::Resize()
	{
		if (m_Specification.Width == 0 || m_Specification.Height == 0)
		{
			XYZ_LOG_WARN("Width and height can not be zero");
			return;
		}

		Renderer::Submit([this]() {
			if (m_RendererID)
			{
				glDeleteFramebuffers(1, &m_RendererID);
				for (auto it : m_ColorAttachments)
					glDeleteTextures(1, &it.RendererID);
				for (auto it : m_DepthAttachments)
					glDeleteTextures(1, &it.RendererID);
			}
			
			if (!m_ColorAttachments.empty() || !m_DepthAttachments.empty())
			{
				glCreateFramebuffers(1, &m_RendererID);
				glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

				bool multisample = m_Specification.Samples > 1;
				if (multisample)
				{
					m_ColorAttachments.resize(1);
					m_DepthAttachments.resize(1);
					setupMultiSample(m_ColorAttachments.back(), m_DepthAttachments.back());
				}
				else
				{
					uint32_t counter = 0;
					for (auto& attachment : m_ColorAttachments)
					{
						setupColorAttachment(attachment, counter);
						counter++;
					}
					for (auto& attachment : m_DepthAttachments)
					{
						setupDepthAttachment(attachment);
					}
					std::vector<uint32_t> attachments;
					for (auto& it : m_ColorAttachments)
						attachments.push_back(it.AttachmentID);
				
					glDrawBuffers(m_ColorAttachments.size(), attachments.data());
				}
				XYZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		});
	}
	void OpenGLFrameBuffer::Bind() const
	{
		Renderer::Submit([this]() {
			XYZ_ASSERT(!m_ColorAttachments.empty() || !m_DepthAttachments.empty(), "Binding framebuffer without any attachments");
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
			});
	}
	void OpenGLFrameBuffer::Unbind() const
	{
		Renderer::Submit([this]() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		});
	}
	
	void OpenGLFrameBuffer::SetSpecification(const FrameBufferSpecs& specs)
	{
		m_Specification = specs;
		Resize();
	}

	void OpenGLFrameBuffer::CreateColorAttachment(FrameBufferFormat format)
	{
		XYZ_ASSERT(m_Specification.Samples == 1, "Cannot create attachments to multisampled frame buffer");
		m_ColorAttachments.push_back({});
		m_ColorAttachments.back().Format = format;
		Renderer::Submit([this, format]() {
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
			ColorAttachment &attachment = m_ColorAttachments.back();
			setupColorAttachment(attachment, m_ColorAttachments.size());
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			});
	}

	void OpenGLFrameBuffer::CreateDepthAttachment()
	{
		XYZ_ASSERT(m_Specification.Samples == 1, "Cannot create attachments to multisampled frame buffer");
		m_DepthAttachments.push_back({});
		Renderer::Submit([this]() {
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
			DepthAttachment &attachment = m_DepthAttachments.back();
			setupDepthAttachment(attachment);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			});
	}

	void OpenGLFrameBuffer::setupColorAttachment(ColorAttachment& attachment, uint32_t index)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &attachment.RendererID);
		glBindTexture(GL_TEXTURE_2D, attachment.RendererID);
		if (attachment.Format == FrameBufferFormat::RGB)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Specification.Width, m_Specification.Height,
				0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		}
		else if (attachment.Format == FrameBufferFormat::RGBA16F)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
		}
		else if (attachment.Format == FrameBufferFormat::RGBA8)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, attachment.RendererID, 0);
		attachment.AttachmentID = GL_COLOR_ATTACHMENT0 + index;
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLFrameBuffer::setupDepthAttachment(DepthAttachment& attachment)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &attachment.RendererID);
		glBindTexture(GL_TEXTURE_2D, attachment.RendererID);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
		);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment.RendererID, 0);
	}
	void OpenGLFrameBuffer::setupMultiSample(ColorAttachment& attachment, DepthAttachment& depth)
	{
		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &attachment.RendererID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachment.RendererID);

		if (m_Specification.Format == FrameBufferFormat::RGBA16F)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_RGBA16F, m_Specification.Width, m_Specification.Height, GL_FALSE);
		}
		else if (m_Specification.Format == FrameBufferFormat::RGBA8)
		{
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_RGBA8, m_Specification.Width, m_Specification.Height, GL_FALSE);
		}
		// glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);


		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &depth.RendererID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth.RendererID);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, GL_FALSE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	}
}