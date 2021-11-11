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
			XYZ_CORE_ERROR("Error: {0} {1} {2}", message, id, severityMessage);
			XYZ_ASSERT("", false);
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			XYZ_CORE_WARN("Deprecated Behaviour: {0} {1} {2}", message, id, severityMessage);
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			XYZ_CORE_ERROR("Undefined Behaviour: {0} {1} {2}", message, id, severityMessage);
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			XYZ_CORE_WARN("Portability: {0} {1} {2}", message, id, severityMessage);
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			XYZ_CORE_WARN("Performance: {0} {1} {2}", message, id, severityMessage);
			break;
		}
	}
	void GLFWErrorCallback(int code, const char* description)
	{
		XYZ_CORE_ERROR("Code: {0} Description: {1} ", code, description);
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
		Ref<OpenGLAPIContext> instance = this;
		Renderer::SubmitAndWait([instance]() {
			glfwMakeContextCurrent(instance->m_WindowHandle);

			XYZ_CORE_INFO("OpenGL Info:");
			XYZ_CORE_INFO("Vendor:   {0}", glGetString(GL_VENDOR));
			XYZ_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
			XYZ_CORE_INFO("Version:  {0}", glGetString(GL_VERSION));

			if (glewInit() != GLEW_OK)
			{
				XYZ_CORE_ERROR("OpenGLContext: Could not initialize glew");
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
		});
	}

	void OpenGLAPIContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}
