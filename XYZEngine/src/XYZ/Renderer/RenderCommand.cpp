#include "stdafx.h"
#include "RenderCommand.h"

namespace XYZ {
	std::unique_ptr<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}