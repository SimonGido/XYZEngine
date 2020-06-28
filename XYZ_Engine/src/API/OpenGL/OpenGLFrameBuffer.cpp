#include "stdafx.h"
#include "OpenGLFrameBuffer.h"
#include "XYZ/Core/Input.h"

#include <GL/glew.h>

namespace XYZ {
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs)
		: m_Specification(specs)
	{
		Resize();
	}
	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		for (auto it : m_ColorAttachments)
			glDeleteTextures(1, &it.RendererID);
		for (auto it : m_DepthAttachments)
			glDeleteTextures(1, &it.RendererID);
	}
	void OpenGLFrameBuffer::Resize()
	{
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
			for (auto& attachment : m_ColorAttachments)
			{
				setupColorAttachment(attachment);
			}
			for (auto attachment : m_DepthAttachments)
			{
				setupDepthAttachment(attachment);
			}
			XYZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	void OpenGLFrameBuffer::Bind() const
	{
		XYZ_ASSERT(m_ColorAttachments.empty() || m_DepthAttachments.empty(), "Binding framebuffer without any attachments");
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}
	void OpenGLFrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	void OpenGLFrameBuffer::SetSpecification(const FrameBufferSpecs& specs)
	{
		m_Specification = specs;
		Resize();
	}

	void OpenGLFrameBuffer::CreateColorAttachment(FrameBufferFormat format)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		ColorAttachment attachment;
		attachment.Format = format;
		setupColorAttachment(attachment);
		m_ColorAttachments.push_back(attachment);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::CreateDepthAttachment()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		DepthAttachment attachment;
		setupDepthAttachment(attachment);
		m_DepthAttachments.push_back(attachment);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::setupColorAttachment(ColorAttachment& attachment)
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size(), GL_TEXTURE_2D, attachment.RendererID, 0);
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


}