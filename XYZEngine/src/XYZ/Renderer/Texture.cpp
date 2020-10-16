#include "stdafx.h"
#include "Texture.h"

#include "Renderer.h"
#include "API/OpenGL/OpenGLTexture.h"

namespace XYZ {
	Ref<Texture2D> Texture2D::Create(const TextureSpecs& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: return Ref<OpenGLTexture2D>::Create(specs);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(TextureWrap wrap, TextureParam min, TextureParam max, const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: return Ref<OpenGLTexture2D>::Create(wrap,min,max, path);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}

	void Texture2D::Bind(uint32_t rendererID, uint32_t slot)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: OpenGLTexture2D::Bind(rendererID, slot); return;
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
	}
	
	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;

	}

}