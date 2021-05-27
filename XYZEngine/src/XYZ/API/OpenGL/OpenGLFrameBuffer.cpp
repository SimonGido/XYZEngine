#include "stdafx.h"
#include "OpenGLFramebuffer.h"

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
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static GLenum DataType(GLenum format)
		{
			switch (format)
			{
			case GL_R32I: return GL_UNSIGNED_BYTE;
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
				switch (format)
				{
				case GL_R32I:
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RED_INTEGER, DataType(format), nullptr);
					break;
				default:
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType(format), nullptr);
					break;
				}

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

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
			case FramebufferTextureFormat::DEPTH32F:
				return true;
			}
			return false;
		}

	}


	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecs& specs)
		: m_Specification(specs)
	{
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
			else
				m_DepthAttachmentFormat = format.TextureFormat;
		}
		Resize(specs.Width, specs.Height, true);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{		
		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() {
			glDeleteFramebuffers(1, &instance->m_RendererID);	
			glDeleteTextures(1, &instance->m_DepthAttachment);
			for (auto it : instance->m_ColorAttachments)
				glDeleteTextures(1, &it);
		});
	}
	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceResize)
	{
		if (width == 0 || height == 0)
		{
			XYZ_LOG_WARN("Width and height can not be zero");
			return;
		}
		if (!forceResize && m_Specification.Width == width && m_Specification.Height == height)
			return;

		m_Specification.Width = width;
		m_Specification.Height = height;

		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			
			if (instance->m_RendererID)
			{
				glDeleteFramebuffers(1, &instance->m_RendererID);
				glDeleteTextures(1, &instance->m_DepthAttachment);
				for (auto it : instance->m_ColorAttachments)
					glDeleteTextures(1, &it);

				instance->m_RendererID = 0;
			}
					
			glGenFramebuffers(1, &instance->m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

			bool multisample = instance->m_Specification.Samples > 1;
			if (instance->m_ColorAttachmentFormats.size())
			{
				instance->m_ColorAttachments.resize(instance->m_ColorAttachmentFormats.size());
				Utils::CreateTextures(multisample, instance->m_ColorAttachments.data(), instance->m_ColorAttachments.size());
				for (int i = 0; i < instance->m_ColorAttachments.size(); i++)
				{
					Utils::BindTexture(multisample, instance->m_ColorAttachments[i]);
					switch (instance->m_ColorAttachmentFormats[i])
					{
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA8, instance->m_Specification.Width, instance->m_Specification.Height, i);
						break;
					case FramebufferTextureFormat::RGBA16F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA16F, instance->m_Specification.Width, instance->m_Specification.Height, i);
						break;
					case FramebufferTextureFormat::RGBA32F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA32F, instance->m_Specification.Width, instance->m_Specification.Height, i);
						break;
					case FramebufferTextureFormat::RG32F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RG32F, instance->m_Specification.Width, instance->m_Specification.Height, i);
						break;
					case FramebufferTextureFormat::R32I:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_R32I, instance->m_Specification.Width, instance->m_Specification.Height, i);
					}
				}
			}
			if (instance->m_DepthAttachmentFormat != FramebufferTextureFormat::None)
			{
				Utils::CreateTextures(multisample, &instance->m_DepthAttachment, 1);
				Utils::BindTexture(multisample, instance->m_DepthAttachment);
				switch (instance->m_DepthAttachmentFormat)
				{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_Specification.Width, instance->m_Specification.Height);
					break;
				case FramebufferTextureFormat::DEPTH32F:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, instance->m_Specification.Width, instance->m_Specification.Height);
					break;
				}
			}

			if (instance->m_ColorAttachmentFormats.size())
			{
				GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
				glDrawBuffers(instance->m_ColorAttachmentFormats.size(), buffers);
			}
			else
			{
				glDrawBuffer(GL_NONE);
			}
			XYZ_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}
	void OpenGLFramebuffer::Bind() const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
			glViewport(0, 0, instance->m_Specification.Width, instance->m_Specification.Height);
		});
	}
	void OpenGLFramebuffer::Unbind() const
	{
		Renderer::Submit([this]() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		});
	}

	void OpenGLFramebuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, attachmentIndex, slot]() {
			glBindTextureUnit(slot, instance->m_ColorAttachments[attachmentIndex]);
		});
	}

	void OpenGLFramebuffer::SetSpecification(const FramebufferSpecs& specs)
	{
		m_Specification = specs;
		//Resize(specs.Width, specs.Height);
	}
	void OpenGLFramebuffer::ReadPixel(int32_t& pixel, uint32_t mx, uint32_t my, uint32_t attachmentIndex) const
	{	
		Renderer::Submit([this, mx, my, attachmentIndex, &pixel]() {
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
			glReadPixels(mx, my, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}
	void OpenGLFramebuffer::ClearColorAttachment(uint32_t colorAttachmentIndex, void* clearValue) const
	{
		// TODO: Make it work with other types than GL_RED_INTEGER
		int clearVal = *(int*)clearValue;
		Renderer::Submit([this, colorAttachmentIndex, clearVal]() {
			glClearTexImage(m_ColorAttachments[colorAttachmentIndex], 0, GL_RED_INTEGER, GL_INT, &clearVal);
		});
	}
}