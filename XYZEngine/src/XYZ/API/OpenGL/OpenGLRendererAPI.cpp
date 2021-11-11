#include "stdafx.h"
#include "OpenGLRendererAPI.h"
#include "XYZ/Renderer/Framebuffer.h"
#include <GL/glew.h>

namespace XYZ {
	
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(2.0f);

		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = (const char*)glGetString(GL_VENDOR);
		caps.Renderer = (const char*)glGetString(GL_RENDERER);
		caps.Version = (const char*)glGetString(GL_VERSION);

		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &caps.MaxTextureUnits);
		glEnable(GL_PROGRAM_POINT_SIZE);
	}
	void OpenGLRendererAPI::SetDepth(bool enabled)
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetScissor(bool enabled)
	{
		if (enabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}

	void OpenGLRendererAPI::SetLineThickness(float thickness)
	{
		glLineWidth(thickness);
	}

	void OpenGLRendererAPI::SetPointSize(float size)
	{
		glPointSize(size);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ReadPixels(uint32_t xCoord, uint32_t yCoord, uint32_t width, uint32_t height, uint8_t* data)
	{
		glReadPixels(xCoord, yCoord, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLRendererAPI::DrawArrays(PrimitiveType type, uint32_t count)
	{
		switch (type)
		{
		case XYZ::PrimitiveType::None:
			XYZ_ASSERT(false, "Primitive type is none");
			break;
		case XYZ::PrimitiveType::Triangles:
			glDrawArrays(GL_TRIANGLES, 0, count);
			break;
		case XYZ::PrimitiveType::Lines:
			glDrawArrays(GL_LINES, 0, count);
			break;
		case XYZ::PrimitiveType::Points:
			glDrawArrays(GL_POINTS, 0, count);
		}

	}

	void OpenGLRendererAPI::DrawIndexed(PrimitiveType type, uint32_t indexCount)
	{
		switch (type)
		{
		case XYZ::PrimitiveType::None:
			XYZ_ASSERT(false, "Primitive type is none");
			break;
		case XYZ::PrimitiveType::Triangles:
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
			break;
		case XYZ::PrimitiveType::Lines:
			glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
			break;
		case XYZ::PrimitiveType::Points:
			glDrawElements(GL_POINTS, indexCount, GL_UNSIGNED_INT, nullptr);
			break;
		}
	}

	void OpenGLRendererAPI::DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset)
	{
		switch (type)
		{
		case XYZ::PrimitiveType::None:
			XYZ_ASSERT(false, "Primitive type is none");
			break;
		case XYZ::PrimitiveType::Triangles:
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instanceCount, offset);
			break;
		case XYZ::PrimitiveType::Lines:
			glDrawElementsInstancedBaseInstance(GL_LINES, indexCount, GL_UNSIGNED_INT, 0, instanceCount, offset);
			break;
		case XYZ::PrimitiveType::Points:
			glDrawElementsInstancedBaseInstance(GL_POINTS, indexCount, GL_UNSIGNED_INT, 0, instanceCount, offset);
			break;
		}
	}
	void OpenGLRendererAPI::DrawInstancedIndirect(void* indirect)
	{
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, indirect);		
	}
}