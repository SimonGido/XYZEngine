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

		static GLenum FormatType(ImageFormat format)
		{
			switch (format)
			{
			case XYZ::ImageFormat::None:
				break;
			case XYZ::ImageFormat::RGBA8:
				return GL_RGBA8;
				break;
			case XYZ::ImageFormat::RGBA16F:
				return GL_RGBA16F;
				break;
			case XYZ::ImageFormat::RGBA32F:
				return GL_RGBA32F;
				break;
			case XYZ::ImageFormat::RG32F:
				return GL_RG32F;
				break;
			case XYZ::ImageFormat::R32I:
				return GL_R32I;
				break;
			case XYZ::ImageFormat::DEPTH32F:
				return GL_DEPTH32F_STENCIL8;
				break;
			case XYZ::ImageFormat::DEPTH24STENCIL8:
				return GL_DEPTH24_STENCIL8;
				break;
			default:
				break;
			}
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

		static bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::DEPTH24STENCIL8:
			case ImageFormat::DEPTH32F:
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
			{
				m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
				m_ColorAttachments.push_back({ 0, format.GenerateMips });
			}
			else
				m_DepthAttachmentFormat = format.TextureFormat;
		}
		Resize(specs.Width, specs.Height, true);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		uint32_t rendererID = m_RendererID;
		uint32_t depthAttachment = m_DepthAttachment;
		auto colorAttachments = m_ColorAttachments;
		Renderer::Submit([rendererID, depthAttachment, colorAttachments]() {

			glDeleteFramebuffers(1, &rendererID);
			glDeleteTextures(1, &depthAttachment);
			for (auto it : colorAttachments)
				glDeleteTextures(1, &it.ID);
		});
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceResize)
	{
		if (width == 0 || height == 0)
		{
			XYZ_CORE_WARN("Width and height can not be zero");
			return;
		}
		if (!forceResize && m_Specification.Width == width && m_Specification.Height == height)
			return;

		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, width, height]() mutable {
			
			instance->m_Specification.Width = width;
			instance->m_Specification.Height = height;
			if (instance->m_RendererID)
			{
				instance->destroyFramebuffer();
				instance->m_RendererID = 0;
			}
					
			glGenFramebuffers(1, &instance->m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

			bool multisample = instance->m_Specification.Samples > 1;
			if (!instance->m_ColorAttachmentFormats.empty())
			{
				for (auto& attachment : instance->m_ColorAttachments)
					glCreateTextures(Utils::TextureTarget(multisample), 1, &attachment.ID);
				
				for (int i = 0; i < instance->m_ColorAttachments.size(); i++)
				{
					auto& attachment = instance->m_ColorAttachments[i];
					Utils::BindTexture(multisample, instance->m_ColorAttachments[i].ID);
					attachColorTexture(attachment, instance->m_Specification.Samples, Utils::FormatType(instance->m_ColorAttachmentFormats[i]), instance->m_Specification.Width, instance->m_Specification.Height, i);
				}
			}
			if (instance->m_DepthAttachmentFormat != ImageFormat::None)
			{
				Utils::CreateTextures(multisample, &instance->m_DepthAttachment, 1);
				Utils::BindTexture(multisample, instance->m_DepthAttachment);
				switch (instance->m_DepthAttachmentFormat)
				{
				case ImageFormat::DEPTH24STENCIL8:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_Specification.Width, instance->m_Specification.Height);
					break;
				case ImageFormat::DEPTH32F:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, instance->m_Specification.Width, instance->m_Specification.Height);
					break;
				}
			}

			if (instance->m_ColorAttachmentFormats.size())
			{
				GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
				glDrawBuffers((GLsizei)instance->m_ColorAttachmentFormats.size(), buffers);
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
		size_t colorAttachmentCount = m_ColorAttachmentFormats.size();
		Renderer::Submit([instance, colorAttachmentCount]() mutable {
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
			glViewport(0, 0, instance->m_Specification.Width, instance->m_Specification.Height);
			
		});
	}
	void OpenGLFramebuffer::Unbind() const
	{
		Renderer::Submit([]() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		});
	}

	void OpenGLFramebuffer::Clear() const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() {
			RendererAPI::SetClearColor(instance->GetSpecification().ClearColor);
			RendererAPI::Clear();
		});
	}

	void OpenGLFramebuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, attachmentIndex, slot]() {
			glBindTextureUnit(slot, instance->m_ColorAttachments[attachmentIndex].ID);
			if (instance->m_ColorAttachments[attachmentIndex].GenerateMips)
				glGenerateMipmap(GL_TEXTURE_2D);
		});
	}

	void OpenGLFramebuffer::BindImage(uint32_t attachmentIndex, uint32_t slot, uint32_t miplevel, BindImageType type) const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, attachmentIndex, slot, miplevel, type]() {
			
			auto& attachment = instance->m_ColorAttachments[attachmentIndex];
			switch (type)
			{
			case XYZ::BindImageType::Read:
				glBindImageTexture(slot, attachment.ID, miplevel, GL_FALSE, 0, GL_READ_ONLY, attachment.InternalFormat);
				break;
			case XYZ::BindImageType::Write:
				glBindImageTexture(slot, attachment.ID, miplevel, GL_FALSE, 0, GL_WRITE_ONLY, attachment.InternalFormat);
				break;
			case XYZ::BindImageType::ReadWrite:
				glBindImageTexture(slot, attachment.ID, miplevel, GL_FALSE, 0, GL_READ_WRITE, attachment.InternalFormat);
				break;
			default:
				break;
			}
		});
	}

	void OpenGLFramebuffer::SetSpecification(const FramebufferSpecs& specs)
	{
		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, specs]() mutable {
			instance->m_Specification = specs;
		});
	}
	void OpenGLFramebuffer::ReadPixel(int32_t& pixel, uint32_t mx, uint32_t my, uint32_t attachmentIndex) const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, mx, my, attachmentIndex, &pixel]() {

			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
			glReadPixels(mx, my, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}
	void OpenGLFramebuffer::ClearColorAttachment(uint32_t colorAttachmentIndex, void* clearValue) const
	{
		// TODO: Make it work with other types than GL_RED_INTEGER
		int clearVal = *(int*)clearValue;
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, colorAttachmentIndex, clearVal]() {
			glClearTexImage(instance->m_ColorAttachments[colorAttachmentIndex].ID, 0, GL_RED_INTEGER, GL_INT, &clearVal);
		});
	}
	void OpenGLFramebuffer::destroyFramebuffer() const
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_DepthAttachment);
		for (auto it : m_ColorAttachments)
			glDeleteTextures(1, &it.ID);
	}
	void OpenGLFramebuffer::attachColorTexture(ColorAttachment& attachment, int samples, GLenum format, uint32_t width, uint32_t height, int index)
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
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RED_INTEGER, Utils::DataType(format), nullptr);
				break;
			default:
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, Utils::DataType(format), nullptr);
				break;
			}
			if (attachment.GenerateMips != 0)
			{
				glTexParameteri(Utils::TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
				glTexParameteri(Utils::TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);


			glTexParameteri(Utils::TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(Utils::TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(Utils::TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		attachment.InternalFormat = format;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, Utils::TextureTarget(multisampled), attachment.ID, 0);
	}
}