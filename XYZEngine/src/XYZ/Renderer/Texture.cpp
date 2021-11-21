#include "stdafx.h"
#include "Texture.h"

#include "Renderer.h"
#include "XYZ/API/OpenGL/OpenGLTexture.h"
#include "XYZ/API/Vulkan/VulkanTexture.h"

namespace XYZ {

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;

	}

	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& properties)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::OpenGL");
		case RendererAPI::API::Vulkan: return Ref<VulkanTexture2D>::Create(format, width, height, data, properties);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, const TextureProperties& properties)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::OpenGL");
		case RendererAPI::API::Vulkan: return Ref<VulkanTexture2D>::Create(path, properties);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}


}