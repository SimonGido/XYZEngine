#include "stdafx.h"
#include "RenderTexture.h"


#include "Renderer.h"
#include "XYZ/API/OpenGL/OpenGLRenderTexture.h"


namespace XYZ {

	Ref<RenderTexture> RenderTexture::Create(const Ref<FrameBuffer>& renderTarget)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		case RendererAPI::API::OpenGL: return Ref<OpenGLRenderTexture>::Create(renderTarget);
		}

		XYZ_ASSERT(false, "Renderer::GetAPI() = RendererAPI::None");
		return nullptr;
	}
}