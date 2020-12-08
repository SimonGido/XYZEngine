#include "stdafx.h"
#include "Buffer.h"
#include "APIContext.h"
#include "XYZ/API/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace XYZ {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(indices, count);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLShaderStorageBuffer>::Create((float*)NULL, size, BufferUsage::Dynamic);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(float* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLShaderStorageBuffer>::Create(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<AtomicCounter> AtomicCounter::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLAtomicCounter>::Create(size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<IndirectBuffer> IndirectBuffer::Create(void* drawCommand, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLIndirectBuffer>::Create(drawCommand, size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
