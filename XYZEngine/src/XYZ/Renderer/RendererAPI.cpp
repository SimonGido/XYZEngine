#include "stdafx.h"
#include "RendererAPI.h"
#include "API/OpenGL/OpenGLRendererAPI.h"

#include <memory>


namespace XYZ {
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	std::unique_ptr<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_unique<OpenGLRendererAPI>();
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}