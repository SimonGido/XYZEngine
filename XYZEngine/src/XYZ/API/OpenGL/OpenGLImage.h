#pragma once
#include "XYZ/Renderer/Image.h"

#include <GL/glew.h>

namespace XYZ {
	class OpenGLImage2D : public Image2D
	{
	public:
		OpenGLImage2D(ImageSpecification specification, ByteBuffer buffer);
		OpenGLImage2D(ImageSpecification specification, const void* data = nullptr);
		virtual ~OpenGLImage2D();

		virtual void Invalidate() override;
		virtual void Release() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual float GetAspectRatio() const override { return (float)m_Specification.Width / (float)m_Specification.Height; }

		virtual ImageSpecification& GetSpecification() override { return m_Specification; }
		virtual const ImageSpecification& GetSpecification() const override { return m_Specification; }

		virtual ByteBuffer GetBuffer() const override { return m_ImageData; }
		virtual ByteBuffer& GetBuffer() override { return m_ImageData; }

		virtual void CreatePerLayerImageViews() override {}

		uint32_t& GetRendererID() { return m_RendererID; }
		uint32_t GetRendererID() const { return m_RendererID; }

		uint32_t& GetSamplerRendererID() { return m_SamplerRendererID; }
		uint32_t GetSamplerRendererID() const { return m_SamplerRendererID; }

		void CreateSampler(const TextureProperties& properties);

		virtual uint64_t GetHash() const override { return (uint64_t)m_RendererID; }
	private:
		void RT_invalidate();
		void RT_release();

	private:
		ImageSpecification m_Specification;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID = 0;
		uint32_t m_SamplerRendererID = 0;

		ByteBuffer m_ImageData;
	};

	namespace Utils {

		inline GLenum OpenGLImageFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB:     return GL_RGB;
			case ImageFormat::SRGB:    return GL_RGB;
			case ImageFormat::RGBA:
			case ImageFormat::RGBA16F:
			case ImageFormat::RGBA32F: return GL_RGBA;
			}
			XYZ_ASSERT(false, "Unknown image format");
			return 0;
		}

		inline GLenum OpenGLImageInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB:             return GL_RGB8;
			case ImageFormat::SRGB:            return GL_SRGB8;
			case ImageFormat::RGBA:            return GL_RGBA8;
			case ImageFormat::RGBA16F:         return GL_RGBA16F;
			case ImageFormat::RGBA32F:         return GL_RGBA32F;
			case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
			case ImageFormat::DEPTH32F:        return GL_DEPTH_COMPONENT32F;
			}
			XYZ_ASSERT(false, "Unknown image format");
			return 0;
		}

		inline GLenum OpenGLFormatDataType(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB:
			case ImageFormat::SRGB:
			case ImageFormat::RGBA:    return GL_UNSIGNED_BYTE;
			case ImageFormat::RGBA16F:
			case ImageFormat::RGBA32F: return GL_FLOAT;
			}
			XYZ_ASSERT(false, "Unknown image format");
			return 0;
		}

		inline GLenum OpenGLSamplerWrap(TextureWrap wrap)
		{
			switch (wrap)
			{
			case TextureWrap::Clamp:   return GL_CLAMP_TO_EDGE;
			case TextureWrap::Repeat:  return GL_REPEAT;
			}
			XYZ_ASSERT(false, "Unknown wrap mode");
			return 0;
		}

		// Note: should always be called with mipmap = false for magnification filtering
		inline GLenum OpenGLSamplerFilter(TextureFilter filter, bool mipmap)
		{
			switch (filter)
			{
			case TextureFilter::Linear:   return mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			case TextureFilter::Nearest:  return mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			}
			XYZ_ASSERT(false, "Unknown filter");
			return 0;
		}

	}

}