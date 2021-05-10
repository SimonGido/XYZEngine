#include "stdafx.h"
#include "OpenGLAPIContext.h"
#include "XYZ/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <csignal>

namespace XYZ {

#ifdef XYZ_DEBUG
	void OpenglCallbackFunction(
		GLenum source,
		GLenum Component,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam)
	{

		std::string severityMessage;
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_LOW:
			severityMessage = "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			severityMessage = "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			severityMessage = "HIGH";
			break;
		}

		switch (Component)
		{
		case GL_DEBUG_TYPE_ERROR:
			XYZ_LOG_API("Error ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			XYZ_ASSERT("", false);
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			XYZ_LOG_API("Deprecated Behaviour: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			XYZ_LOG_API("Undefined Behaviour: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			XYZ_LOG_API("Portability: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			XYZ_LOG_API("Performance: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_OTHER:
			XYZ_LOG_API(message);
			XYZ_LOG_API("ID: ", id);
			break;
		}
	}
	void GLFWErrorCallback(int code, const char* description)
	{
		XYZ_LOG_ERR(code, " ", description);
	}
#endif

	OpenGLAPIContext::OpenGLAPIContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		XYZ_ASSERT(windowHandle, "Window handle is null!");
	}

	OpenGLAPIContext::~OpenGLAPIContext()
	{
	}

	void OpenGLAPIContext::Init()
	{		
		glfwMakeContextCurrent(m_WindowHandle);
		
		XYZ_LOG_WARN("OpenGL Info:");
		XYZ_LOG_WARN("Vendor: ", (char*)glGetString(GL_VENDOR));
		XYZ_LOG_WARN("Renderer: ", (char*)glGetString(GL_RENDERER));
		XYZ_LOG_WARN("Version: ", (char*)glGetString(GL_VERSION));

		if (glewInit() != GLEW_OK)
		{
			XYZ_LOG_ERR("OpenGLContext: Could not initialize glew");
		};
		glLoadIdentity();


#ifdef XYZ_DEBUG
		if (glDebugMessageCallback)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glEnable(GL_DEBUG_OUTPUT);

			glDebugMessageCallback(OpenglCallbackFunction, nullptr);
			GLuint unusedIds = 0;
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
		}
		glfwSetErrorCallback(GLFWErrorCallback);
#endif
	}

	void OpenGLAPIContext::SwapBuffers()
	{
		//Renderer::Submit([&] {
		//	glFinish();
		//});
		glfwSwapBuffers(m_WindowHandle);
	}

}
