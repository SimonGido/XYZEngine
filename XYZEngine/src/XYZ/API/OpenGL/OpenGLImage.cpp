#include "stdafx.h"
#include "OpenGLImage.h"

#include "XYZ/Renderer/Renderer.h"

#include <GL/glew.h>


namespace XYZ {

	OpenGLImage2D::OpenGLImage2D(ImageSpecification specification, const void* data)
		: m_Specification(specification), m_Width(specification.Width), m_Height(specification.Height)
	{
		if (data)
			m_ImageData = ByteBuffer::Copy(data, Utils::GetImageMemorySize(specification.Format, specification.Width, specification.Height));
	}

	OpenGLImage2D::OpenGLImage2D(ImageSpecification specification, ByteBuffer buffer)
		: m_Specification(specification), m_Width(specification.Width), m_Height(specification.Height), m_ImageData(buffer)
	{
	}

	OpenGLImage2D::~OpenGLImage2D()
	{
		m_ImageData.Destroy();
		if (m_RendererID)
		{
			uint32_t rendererID = m_RendererID;
			Renderer::Submit([rendererID]()
			{
				glDeleteTextures(1, &rendererID);
			});
		}
	}

	void OpenGLImage2D::Invalidate()
	{
		Ref<OpenGLImage2D> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_invalidate();
		});
		
	}

	void OpenGLImage2D::Release()
	{
		Ref<OpenGLImage2D> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_release();
		});	
	}

	void OpenGLImage2D::CreateSampler(const TextureProperties& properties)
	{
		Ref<OpenGLImage2D> instance = this;
		Renderer::Submit([instance, properties]() mutable {
			glCreateSamplers(1, &instance->m_SamplerRendererID);
			glSamplerParameteri(instance->m_SamplerRendererID, GL_TEXTURE_MIN_FILTER, Utils::OpenGLSamplerFilter(properties.SamplerFilter, properties.GenerateMips));
			glSamplerParameteri(instance->m_SamplerRendererID, GL_TEXTURE_MAG_FILTER, Utils::OpenGLSamplerFilter(properties.SamplerFilter, false));
			glSamplerParameteri(instance->m_SamplerRendererID, GL_TEXTURE_WRAP_R, Utils::OpenGLSamplerWrap(properties.SamplerWrap));
			glSamplerParameteri(instance->m_SamplerRendererID, GL_TEXTURE_WRAP_S, Utils::OpenGLSamplerWrap(properties.SamplerWrap));
			glSamplerParameteri(instance->m_SamplerRendererID, GL_TEXTURE_WRAP_T, Utils::OpenGLSamplerWrap(properties.SamplerWrap));
		});
	}

	void OpenGLImage2D::RT_invalidate()
	{
		if (m_RendererID)
			RT_release();

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		GLenum internalFormat = Utils::OpenGLImageInternalFormat(m_Specification.Format);
		uint32_t mipCount = Utils::CalculateMipCount(m_Width, m_Height);
		glTextureStorage2D(m_RendererID, mipCount, internalFormat, m_Width, m_Height);
		if (m_ImageData)
		{
			GLenum format = Utils::OpenGLImageFormat(m_Specification.Format);
			GLenum dataType = Utils::OpenGLFormatDataType(m_Specification.Format);
			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, format, dataType, m_ImageData);
			glGenerateTextureMipmap(m_RendererID); // TODO: optional
		}
	}

	void OpenGLImage2D::RT_release()
	{
		
		if (m_RendererID)
		{
			glDeleteTextures(1, &m_RendererID);
			m_RendererID = 0;
		}
		m_ImageData.Destroy();
	}

}