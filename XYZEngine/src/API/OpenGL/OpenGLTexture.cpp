#include "stdafx.h"
#include "OpenGLTexture.h" 

#include <stb_image.h>


namespace XYZ {
	OpenGLTexture2D::OpenGLTexture2D(TextureWrap wrap, TextureParam min, TextureParam max, const std::string& path)
	{
		m_Filepath = path;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		XYZ_ASSERT(data, "Failed to load image!");
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

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		int levels = Texture::CalculateMipMapCount(m_Specification.Width, m_Specification.Height);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Specification.Width, m_Specification.Height);
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

		if (wrap == TextureWrap::Repeat)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (wrap == TextureWrap::Clamp)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width, m_Specification.Height, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(m_RendererID);
		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecs& specs)
	{
		m_Specification.Width = specs.Width;
		m_Specification.Height = specs.Height;
		m_Specification.Wrap = specs.Wrap;
		m_Specification.Format = specs.Format;

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Specification.Width, m_Specification.Height);

		if (specs.MinParam == TextureParam::Linear)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if (specs.MinParam == TextureParam::Nearest)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		if (specs.MagParam == TextureParam::Linear)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (specs.MagParam == TextureParam::Nearest)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if (specs.Wrap == TextureWrap::Repeat)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (specs.Wrap == TextureWrap::Clamp)
		{
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		XYZ_ASSERT(size == m_Specification.Width * m_Specification.Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	uint8_t* OpenGLTexture2D::GetData()
	{
		uint8_t* buffer = new uint8_t[m_Specification.Width * m_Specification.Height * m_Specification.Channels * sizeof(uint8_t)];
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glGetTexImage(GL_TEXTURE_2D, 0, m_DataFormat, GL_UNSIGNED_BYTE, buffer);

		return buffer;
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t rendererID,uint32_t slot)
	{
		glBindTextureUnit(slot, rendererID);
	}


}