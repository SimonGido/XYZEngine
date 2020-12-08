#include "stdafx.h"
#include "OpenGLFrameBuffer.h"

#include "XYZ/Renderer/Renderer.h"

#include <GL/glew.h>

namespace XYZ {
	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), 1, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static GLenum DataType(GLenum format)
		{
			switch (format)
			{
			case GL_RGBA8: return GL_UNSIGNED_BYTE;
			case GL_RG16F:
			case GL_RG32F:
			case GL_RGBA16F:
			case GL_RGBA32F: return GL_FLOAT;
			case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
			}

			XYZ_ASSERT(false, "Unknown format!");
			return 0;
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				// Only RGBA access for now
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType(format), nullptr);

				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
			case FrameBufferTextureFormat::DEPTH32F:
				return true;
			}
			return false;
		}

	}


	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs)
		: m_Specification(specs)
	{
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
			else
				m_DepthAttachmentFormat = format.TextureFormat;
		}
		Resize(specs.Width, specs.Height);
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{		
		Renderer::Submit([this]() {
			glDeleteFramebuffers(1, &m_RendererID);	
			glDeleteTextures(1, &m_DepthAttachment);
			for (auto it : m_ColorAttachments)
				glDeleteTextures(1, &it);
		});
	}
	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (m_Specification.Width == 0 || m_Specification.Height == 0)
		{
			XYZ_LOG_WARN("Width and height can not be zero");
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;

		Renderer::Submit([this]() {
			
			if (m_RendererID)
			{
				glDeleteFramebuffers(1, &m_RendererID);
				glDeleteTextures(1, &m_DepthAttachment);
				for (auto it : m_ColorAttachments)
					glDeleteTextures(1, &it);
				m_RendererID = 0;
			}
					
			glGenFramebuffers(1, &m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

			bool multisample = m_Specification.Samples > 1;
			if (m_ColorAttachmentFormats.size())
			{
				m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
				Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());
				for (int i = 0; i < m_ColorAttachments.size(); i++)
				{
					Utils::BindTexture(multisample, m_ColorAttachments[i]);
					switch (m_ColorAttachmentFormats[i])
					{
					case FrameBufferTextureFormat::RGBA8:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, m_Specification.Width, m_Specification.Height, i);
						break;
					case FrameBufferTextureFormat::RGBA16F:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA16F, m_Specification.Width, m_Specification.Height, i);
						break;
					case FrameBufferTextureFormat::RGBA32F:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA32F, m_Specification.Width, m_Specification.Height, i);
						break;
					case FrameBufferTextureFormat::RG32F:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RG32F, m_Specification.Width, m_Specification.Height, i);
						break;
					}
				}
			}
			if (m_DepthAttachmentFormat != FrameBufferTextureFormat::None)
			{
				Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
				Utils::BindTexture(multisample, m_DepthAttachment);
				switch (m_DepthAttachmentFormat)
				{
				case FrameBufferTextureFormat::DEPTH24STENCIL8:
					Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
				case FrameBufferTextureFormat::DEPTH32F:
					Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
				}
			}
			XYZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}
	void OpenGLFrameBuffer::Bind() const
	{
		Renderer::Submit([this]() {
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		});
	}
	void OpenGLFrameBuffer::Unbind() const
	{
		Renderer::Submit([this]() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		});
	}

	void OpenGLFrameBuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
	{
		Renderer::Submit([this, attachmentIndex, slot]() {
			glBindTextureUnit(slot, m_ColorAttachments[attachmentIndex]);
		});
	}

	void OpenGLFrameBuffer::SetSpecification(const FrameBufferSpecs& specs)
	{
		m_Specification = specs;
		//Resize(specs.Width, specs.Height);
	}
}