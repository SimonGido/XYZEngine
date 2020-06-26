#include "stdafx.h"
#include "OpenGLAPIContext.h"

#include <GLFW\glfw3.h>
#include <GL\glew.h>
#include <GL/GL.h>
#include <csignal>

namespace XYZ {

#ifdef XYZ_DEBUG
	void APIENTRY openglCallbackFunction(
		GLenum source,
		GLenum type,
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

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
			XYZ_LOG_API("ERROR ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			XYZ_ASSERT("", false);
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			XYZ_LOG_API("DEPRECATED_BEHAVIOR: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			XYZ_LOG_API("UNDEFINED_BEHAVIOR: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			XYZ_LOG_API("PORTABILITY: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			XYZ_LOG_API("PERFORMANCE: ", message);
			XYZ_LOG_API("ID: ", id);
			XYZ_LOG_API("Severity: ", severityMessage);
			break;
		case GL_DEBUG_TYPE_OTHER:
			XYZ_LOG_API(message);
			XYZ_LOG_API("ID: ", id);
			break;
		}
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
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

			glDebugMessageCallback(openglCallbackFunction, nullptr);
			GLuint unusedIds = 0;
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
		}
#endif
	}

	void OpenGLAPIContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}
