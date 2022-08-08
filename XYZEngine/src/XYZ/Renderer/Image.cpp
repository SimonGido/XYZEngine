#include "stdafx.h"
#include "Image.h"

#include "XYZ/API/OpenGL/OpenGLImage.h"
#include "XYZ/API/Vulkan/VulkanImage.h"

#include "Renderer.h"


namespace XYZ {
	Ref<Image2D> Image2D::Create(const ImageSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:
		{
			XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
			return nullptr;
		}
		case RendererAPI::Type::OpenGL: return   Ref<OpenGLImage2D>::Create(specification);
		case RendererAPI::Type::Vulkan: return   Ref<VulkanImage2D>::Create(specification);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}

}