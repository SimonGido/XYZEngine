#include "stdafx.h"
#include "Buffer.h"
#include "APIContext.h"
#include "API/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace XYZ {
	std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::shared_ptr<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLIndexBuffer>(indices, count);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLShaderStorageBuffer>((float*)NULL, size, BufferUsage::Dynamic);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<ShaderStorageBuffer> ShaderStorageBuffer::Create(float* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLShaderStorageBuffer>(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::shared_ptr<AtomicCounter> AtomicCounter::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLAtomicCounter>(size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::shared_ptr<IndirectBuffer> IndirectBuffer::Create(void* drawCommand, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLIndirectBuffer>(drawCommand, size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
