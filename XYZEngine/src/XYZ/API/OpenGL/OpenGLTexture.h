#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Utils/DataStructures/Queue.h"

#include <GL/glew.h>

namespace XYZ {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecs& specs, const std::string& path);
		OpenGLTexture2D(ImageFormat format, uint32_t width, uint32_t height, const TextureSpecs& specs);
		virtual ~OpenGLTexture2D() override;
		virtual void Release() const override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const override;


		virtual void SetData(void* data, uint32_t size) override;
		virtual void GetData(uint8_t** buffer) const override;

		inline virtual uint32_t GetWidth() const override { return m_Width; }
		inline virtual uint32_t GetHeight() const override { return m_Height; }
		inline virtual uint32_t GetChannelSize() const override { return m_ChannelSize; }
		virtual uint32_t		GetMipLevelCount() const override;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t index) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const TextureSpecs& GetSpecification() const override { return m_Specification; };
		virtual const std::string& GetFilepath() const override { return m_Filepath; }
		static void Bind(uint32_t rendererID, uint32_t slot);
	private:
		uint32_t m_RendererID = 0;

		uint32_t m_Width, m_Height;
		uint32_t m_ChannelSize;

		TextureSpecs m_Specification;
		ImageFormat	 m_ImageFormat;
		GLenum m_DataFormat, m_InternalFormat;
		Queue<ByteBuffer> m_Buffers;

		std::string m_Filepath;
	};



	class OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(const TextureSpecs& specs, const std::initializer_list<std::string>& paths);
		OpenGLTexture2DArray(uint32_t layerCount, uint32_t width, uint32_t height, uint32_t channels, const TextureSpecs& specs);
		virtual ~OpenGLTexture2DArray() override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void BindImage(uint32_t slot, uint32_t miplevel, BindImageType type) const override;

		inline virtual uint32_t GetWidth() const override { return m_Width; }
		inline virtual uint32_t GetHeight() const override { return m_Height; }
		inline virtual uint32_t GetChannelSize() const override { return m_Channels; }
		virtual uint32_t		GetMipLevelCount() const override;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t index) const override;
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const TextureSpecs& GetSpecification() const override { return m_Specification; };

	private:
		uint32_t m_RendererID = 0;

		uint32_t m_Width, m_Height;
		uint32_t m_Channels;
		uint32_t m_LayerCount;

		TextureSpecs m_Specification;

		GLenum m_DataFormat, m_InternalFormat;
		Queue<ByteBuffer> m_Buffers;

		std::vector<std::string> m_Filepaths;
	};
}