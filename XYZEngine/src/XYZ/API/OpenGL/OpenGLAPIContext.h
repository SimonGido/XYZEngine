#pragma once
#include "XYZ/Renderer/APIContext.h"


	struct GLFWwindow;
namespace XYZ {


	class OpenGLAPIContext : public APIContext
	{
	public:
		OpenGLAPIContext(GLFWwindow* windowHandle);
		~OpenGLAPIContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;

		bool anotherWin = false;
	};

}

