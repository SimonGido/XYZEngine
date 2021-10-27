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
	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(indices, count);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLShaderStorageBuffer>::Create((float*)NULL, size, binding, BufferUsage::Dynamic);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(const float* vertices, uint32_t size, uint32_t binding, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLShaderStorageBuffer>::Create(vertices, size, binding, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<AtomicCounter> AtomicCounter::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLAtomicCounter>::Create(size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<IndirectBuffer> IndirectBuffer::Create(void* drawCommand, uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLIndirectBuffer>::Create(drawCommand, size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return Ref<OpenGLUniformBuffer>::Create(size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
