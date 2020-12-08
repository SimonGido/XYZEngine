#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <GL/glew.h>

namespace XYZ {
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecs& specs, const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height, uint32_t channels, const TextureSpecs& specs);
		virtual ~OpenGLTexture2D();

	
		virtual void Bind(uint32_t slot = 0) const override;
		virtual void SetData(void* data, uint32_t size) override;
		virtual uint8_t* GetData() override;
		inline virtual uint32_t GetWidth() const override { return m_Width; }
		inline virtual uint32_t GetHeight() const override { return m_Height; }
		inline virtual uint32_t GetChannels() const override { return m_Channels; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const TextureSpecs& GetSpecification() const override { return m_Specification; };

		static void Bind(uint32_t rendererID, uint32_t slot);
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Width, m_Height;
		uint32_t m_Channels;

		TextureSpecs m_Specification;
		
		GLenum m_DataFormat, m_InternalFormat;
		ByteBuffer m_LocalData;
	};	
}