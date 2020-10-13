#pragma once

#include "XYZ/Renderer/Texture.h"
#include <GL/glew.h>

namespace XYZ {
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(TextureWrap wrap, TextureParam min, TextureParam max, const std::string& path);
		OpenGLTexture2D(const TextureSpecs& specs);


		virtual ~OpenGLTexture2D();

		inline virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Specification.Height; }
		inline virtual uint32_t GetChannels() const override { return m_Specification.Channels; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual uint8_t* GetData() override;
		virtual const TextureSpecs& GetSpecification() const { return m_Specification; };
		virtual void Bind(uint32_t slot = 0) const override;

		static void Bind(uint32_t rendererID, uint32_t slot);
	private:
		uint32_t m_RendererID;

		TextureSpecs m_Specification;
		GLenum m_DataFormat, m_InternalFormat;
	};

}