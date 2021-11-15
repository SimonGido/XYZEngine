#pragma once
#include "XYZ/Renderer/APIContext.h"
#include "OpenGLRenderCommandBuffer.h"

struct GLFWwindow;
namespace XYZ {

	class OpenGLAPIContext : public APIContext
	{
	public:
		OpenGLAPIContext();
		virtual ~OpenGLAPIContext() override;

		virtual void Init(GLFWwindow* window) override;
		virtual void SwapBuffers() override;
		virtual Ref<RenderCommandBuffer> GetRenderCommandBuffer() override;
	private:
		GLFWwindow* m_WindowHandle;
		Ref<OpenGLRenderCommandBuffer> m_RenderCommandBuffer;
	};

}

