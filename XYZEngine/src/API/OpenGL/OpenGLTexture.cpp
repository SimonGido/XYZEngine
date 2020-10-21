#include "stdafx.h"
#include "OpenGLTexture.h" 

#include "XYZ/Renderer/Renderer.h"

#include <stb_image.h>


namespace XYZ {
	OpenGLTexture2D::OpenGLTexture2D(TextureWrap wrap, TextureParam min, TextureParam max, const std::string& path)
	{
		m_Filepath = path;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);	
		m_LocalData = (uint8_t*)stbi_load(path.c_str(), &width, &height, &channels, 0);

		XYZ_ASSERT(m_LocalData, "Failed to load image!");
		m_Specification.Width = width;
		m_Specification.Height = height;
		m_Specification.Channels = channels;
		m_Specification.Wrap = wrap;
		m_Specification.MinParam = min;
		m_Specification.MagParam = max;
	
		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 1)
		{
			internalFormat = GL_R8;
			dataFormat = GL_RED;
		}


		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		XYZ_ASSERT(internalFormat & dataFormat, "Format not supported!");

		Renderer::Submit([=]() {
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			int levels = Texture::CalculateMipMapCount(m_Specification.Width, m_Specification.Height);
			glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Specification.Width, m_Specification.Height);
			if (m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			}
			else if (m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			if (m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_DataFormat, GL_UNSIGNED_BYTE, m_LocalData);
			glGenerateTextureMipmap(m_RendererID);
			stbi_image_free(m_LocalData);
			m_LocalData = nullptr;
		});
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecs& specs)
	{
		m_Specification.Width = specs.Width;
		m_Specification.Height = specs.Height;
		m_Specification.Wrap = specs.Wrap;
		m_Specification.Channels = specs.Channels;
		m_DataFormat = 0;
		m_InternalFormat = 0;

		if (specs.Channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (specs.Channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else if (specs.Channels == 1)
		{
			m_InternalFormat = GL_R8;
			m_DataFormat = GL_RED;
		}
		else
		{
			XYZ_ASSERT("Channel is not supported ", specs.Channels);
		}
		
		m_LocalData.Allocate(m_Specification.Width * m_Specification.Height * m_Specification.Channels);

		Renderer::Submit([this]() {

			if (m_Specification.Channels == 1)
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Specification.Width, m_Specification.Height);

			if (m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			else if (m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			if (m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		delete[]m_LocalData;
		Renderer::Submit([=]() {glDeleteTextures(1, &m_RendererID); });
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		m_LocalData.ZeroInitialize();
		m_LocalData.Write(data, size, 0);
		Renderer::Submit([this,size]() {
			XYZ_ASSERT(size == m_Specification.Width * m_Specification.Height * m_Specification.Channels, "Data must be entire texture!");
			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_DataFormat, GL_UNSIGNED_BYTE, m_LocalData);
		});
	}

	uint8_t* OpenGLTexture2D::GetData()
	{
		m_LocalData.Allocate(m_Specification.Width * m_Specification.Height * m_Specification.Channels);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glGetTexImage(GL_TEXTURE_2D, 0, m_DataFormat, GL_UNSIGNED_BYTE, m_LocalData);

		return m_LocalData;
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		Renderer::Submit([this,slot ]() {glBindTextureUnit(slot, m_RendererID); });
	}

	void OpenGLTexture2D::Bind(uint32_t rendererID,uint32_t slot)
	{
		Renderer::Submit([=]() {glBindTextureUnit(slot, rendererID); });
	}


}