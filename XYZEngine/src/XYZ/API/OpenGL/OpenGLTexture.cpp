#include "stdafx.h"
#include "OpenGLTexture.h" 

#include "XYZ/Renderer/Renderer.h"

#include <stb_image.h>


namespace XYZ {
	namespace Utils {
		static uint32_t ImageFormatToSize(ImageFormat format)
		{
			switch (format)
			{
			case XYZ::ImageFormat::None:
				break;
			case XYZ::ImageFormat::RED:
				return 1;
				break;
			case XYZ::ImageFormat::RED32F:
				return 1 * 4;
				break;
			case XYZ::ImageFormat::RGB:
				return 3 * 1;
				break;
			case XYZ::ImageFormat::RGBA8:
				return 4 * 1;
				break;
			case XYZ::ImageFormat::RGBA16F:
				return 4 * 2;
				break;
			case XYZ::ImageFormat::RGBA32F:
				return 4 * 4;
				break;
			case XYZ::ImageFormat::RG32F:
				return 2 * 4;
				break;
			case XYZ::ImageFormat::R32I:
				return 1 * 4;
				break;
			case XYZ::ImageFormat::SRGB:
				return 4 * 1;
				break;
			case XYZ::ImageFormat::DEPTH32F:
				return 1 * 4;
				break;
			case XYZ::ImageFormat::DEPTH24STENCIL8:
				return 1 * 4;
				break;
			}
			return 0;
		}
		static GLenum ImageFormatToOpenGLFormat(ImageFormat format)
		{
			switch (format)
			{
			case XYZ::ImageFormat::None:
				break;
			case XYZ::ImageFormat::RED:
				return GL_RED;
				break;
			case XYZ::ImageFormat::RED32F:
				return GL_R32F;
				break;
			case XYZ::ImageFormat::RGB:
				return GL_RGB8;
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
			case XYZ::ImageFormat::SRGB:
				return GL_SRGB;
				break;
			case XYZ::ImageFormat::DEPTH32F:
				return GL_DEPTH;
				break;
			case XYZ::ImageFormat::DEPTH24STENCIL8:
				return GL_DEPTH24_STENCIL8;
				break;
			}
			return 0;
		}
	}
	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecs& specs, const std::string& path)
		:
		m_Specification(specs),
		m_Filepath(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		ByteBuffer buffer;
		buffer = (uint8_t*)stbi_load(path.c_str(), &width, &height, &channels, 0);


		XYZ_ASSERT(buffer, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		m_ChannelSize = channels;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			m_ImageFormat = ImageFormat::RGBA8;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			m_ImageFormat = ImageFormat::RGB;
		}
		else if (channels == 1)
		{
			internalFormat = GL_R8;
			dataFormat = GL_RED;
			m_ImageFormat = ImageFormat::RED;
		}
		else
		{
			m_ImageFormat = ImageFormat::None;
		}


		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		XYZ_ASSERT(internalFormat & dataFormat, "Format not supported!");
		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, buffer]() mutable {
			glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_RendererID);
			const int levels = Texture::CalculateMipMapCount(instance->m_Width, instance->m_Height);

			glTextureStorage2D(instance->m_RendererID, levels, instance->m_InternalFormat, instance->m_Width, instance->m_Height);
			if (instance->m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAX_LEVEL, levels);
			}
			else if (instance->m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (instance->m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (instance->m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, instance->m_DataFormat, GL_UNSIGNED_BYTE, buffer);

			glGenerateTextureMipmap(instance->m_RendererID);

			stbi_image_free(buffer);
		});
	}

	

	OpenGLTexture2D::OpenGLTexture2D(ImageFormat format, uint32_t width, uint32_t height, const TextureSpecs& specs)
		: m_ImageFormat(format), m_Width(width), m_Height(height), m_Specification(specs)
	{
		m_DataFormat = 0;
		m_InternalFormat = Utils::ImageFormatToOpenGLFormat(m_ImageFormat);
		m_ChannelSize = Utils::ImageFormatToSize(m_ImageFormat);
		if (m_ChannelSize >= 4)
		{
			m_DataFormat = GL_RGBA;
		}
		else if (m_ChannelSize == 3)
		{
			m_DataFormat = GL_RGB;
		}
		else if (m_ChannelSize == 1)
		{
			m_DataFormat = GL_RED;
		}

		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance]() mutable {

			if (instance->m_ChannelSize == 1)
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_RendererID);
			const int levels = Texture::CalculateMipMapCount(instance->m_Width, instance->m_Height);

			
			if (instance->m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			}
			else if (instance->m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			if (instance->m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (instance->m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (instance->m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glTextureStorage2D(instance->m_RendererID, levels, instance->m_InternalFormat, instance->m_Width, instance->m_Height);

			glGenerateTextureMipmap(instance->m_RendererID);
		});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[] buffer;
		}	
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteTextures(1, &rendererID);
		});
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Width * m_Height * m_ChannelSize);
		else
			buffer = m_Buffers.PopBack();

		buffer.ZeroInitialize();
		buffer.Write(data, size);
		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, size, buffer]() mutable {
			XYZ_ASSERT(size == instance->m_Width * instance->m_Height * instance->m_ChannelSize, "Data must be entire texture!");
			XYZ_ASSERT(instance->m_DataFormat && instance->m_InternalFormat, "Texture has no format or was created from frame buffer");
			glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, instance->m_DataFormat, GL_UNSIGNED_BYTE, buffer);
			instance->m_Buffers.EmplaceBack(std::move(buffer));
		});
	}

	void OpenGLTexture2D::GetData(uint8_t** buffer) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, buffer]() mutable {
			const size_t size = (size_t)instance->m_Width * (size_t)instance->m_Height * (size_t)instance->m_ChannelSize;
			*buffer = new uint8_t[size];

			glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);
			glGetTexImage(GL_TEXTURE_2D, 0, instance->m_DataFormat, GL_UNSIGNED_BYTE, *buffer);
		});
	}

	uint32_t OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	std::pair<uint32_t, uint32_t> OpenGLTexture2D::GetMipSize(uint32_t index) const
	{
		uint32_t width = m_Width;
		uint32_t height = m_Height;
		while (index != 0)
		{
			width /= 2;
			height /= 2;
			index--;
		}

		return { width, height };

	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, slot]() {
			glBindTextureUnit(slot, instance->m_RendererID);
		});
	}

	void OpenGLTexture2D::BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, slot, miplevel, type]() {
			switch (type)
			{
			case XYZ::BindImageType::Read:
				glBindImageTexture(slot, instance->m_RendererID, miplevel, GL_FALSE, 0, GL_READ_ONLY, instance->m_InternalFormat);
				break;
			case XYZ::BindImageType::Write:
				glBindImageTexture(slot, instance->m_RendererID, miplevel, GL_FALSE, 0, GL_WRITE_ONLY, instance->m_InternalFormat);
				break;
			case XYZ::BindImageType::ReadWrite:
				glBindImageTexture(slot, instance->m_RendererID, miplevel, GL_FALSE, 0, GL_READ_WRITE, instance->m_InternalFormat);
				break;
			default:
				break;
			}		
		});
	}


	void OpenGLTexture2D::Bind(uint32_t rendererID, uint32_t slot)
	{
		Renderer::Submit([=]() {	
			glBindTextureUnit(slot, rendererID);		
		});
	}
	OpenGLTexture2DArray::OpenGLTexture2DArray(const TextureSpecs& specs, const std::initializer_list<std::string>& paths)
		:
		m_Specification(specs),
		m_Filepaths(paths),
		m_LayerCount((uint32_t)paths.size())
	{
		XYZ_ASSERT(paths.size(), "No paths");
		int width = 0, height = 0, channels = 0;
		stbi_set_flip_vertically_on_load(1);
		uint32_t offset = 0;

		ByteBuffer buffer;
		buffer.Allocate(width * height * channels * paths.size());
		for (auto& path : m_Filepaths)
		{
			const uint8_t* ptr = (uint8_t*)stbi_load(path.c_str(), &width, &height, &channels, 0);
			const uint32_t size = width * height * channels;

			buffer.Write(ptr, size, offset);
			offset += size;
		}
		XYZ_ASSERT(buffer, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		m_Channels = channels;

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
		Ref<OpenGLTexture2DArray> instance = this;
		Renderer::Submit([instance, buffer]() mutable {
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &instance->m_RendererID);
			const int levels = Texture::CalculateMipMapCount(instance->m_Width, instance->m_Height);
			glTextureStorage3D(instance->m_RendererID, 1, instance->m_InternalFormat, instance->m_Width, instance->m_Height, instance->m_LayerCount);
			if (instance->m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			}
			else if (instance->m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (instance->m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (instance->m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glTextureSubImage3D(instance->m_RendererID, 0, 0, 0, 0, instance->m_Width, instance->m_Height, instance->m_LayerCount, instance->m_DataFormat, GL_UNSIGNED_BYTE, buffer);
			glGenerateTextureMipmap(instance->m_RendererID);

			stbi_image_free(buffer);
		});
	}
	OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t layerCount, uint32_t width, uint32_t height, uint32_t channels, const TextureSpecs& specs)
		: m_LayerCount(layerCount), m_Width(width), m_Height(height), m_Channels(channels), m_Specification(specs)
	{
		m_DataFormat = 0;
		m_InternalFormat = 0;

		if (m_Channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (m_Channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else if (m_Channels == 1)
		{
			m_InternalFormat = GL_R8;
			m_DataFormat = GL_RED;
		}
		else
		{
			XYZ_ASSERT("Channel is not supported ", m_Channels);
		}

		ByteBuffer buffer;
		buffer.Allocate(m_Width * m_Height * m_Channels * m_LayerCount);

		Ref<OpenGLTexture2DArray> instance = this;
		Renderer::Submit([instance]() mutable {

			if (instance->m_Channels == 1)
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &instance->m_RendererID);
			glTextureStorage3D(instance->m_RendererID, 1, instance->m_InternalFormat, instance->m_Width, instance->m_Height, instance->m_LayerCount);

			if (instance->m_Specification.MinParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			else if (instance->m_Specification.MinParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			if (instance->m_Specification.MagParam == TextureParam::Linear)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else if (instance->m_Specification.MagParam == TextureParam::Nearest)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (instance->m_Specification.Wrap == TextureWrap::Repeat)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else if (instance->m_Specification.Wrap == TextureWrap::Clamp)
			{
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		});
	}
	OpenGLTexture2DArray::~OpenGLTexture2DArray()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[] buffer;
		}
		Renderer::Submit([=]() {glDeleteTextures(1, &m_RendererID); });
	}
	void OpenGLTexture2DArray::Bind(uint32_t slot) const
	{
		Ref<const OpenGLTexture2DArray> instance = this;
		Renderer::Submit([instance, slot]() {
			glBindTextureUnit(slot, instance->m_RendererID);
		});
	}
	void OpenGLTexture2DArray::BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const
	{
	}
	uint32_t OpenGLTexture2DArray::GetMipLevelCount() const
	{
		return uint32_t();
	}
	std::pair<uint32_t, uint32_t> OpenGLTexture2DArray::GetMipSize(uint32_t index) const
	{
		return std::pair<uint32_t, uint32_t>();
	}
}