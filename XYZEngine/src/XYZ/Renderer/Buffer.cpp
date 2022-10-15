#include "stdafx.h"
#include "Buffer.h"
#include "APIContext.h"
#include "XYZ/API/OpenGL/OpenGLBuffer.h"
#include "XYZ/API/Vulkan/VulkanVertexBuffer.h"
#include "XYZ/API/Vulkan/VulkanIndexBuffer.h"
#include "XYZ/API/Vulkan/VulkanUniformBuffer.h"
#include "XYZ/API/Vulkan/VulkanStorageBuffer.h"
#include "XYZ/API/Vulkan/VulkanIndirectBuffer.h"

#include "Renderer.h"

namespace XYZ {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(size);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanVertexBuffer>::Create(size);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(vertices, size, usage);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanVertexBuffer>::Create(vertices, size, usage);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const void* indices, uint32_t count, IndexType type)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(indices, count, type);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanIndexBuffer>::Create(indices, count, type);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLStorageBuffer>::Create((float*)NULL, size, binding, BufferUsage::Dynamic);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanStorageBuffer>::Create(size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<StorageBuffer> StorageBuffer::Create(const void* data, uint32_t size, uint32_t binding, BufferUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLStorageBuffer>::Create(data, size, binding, usage);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanStorageBuffer>::Create(data, size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<AtomicCounter> AtomicCounter::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLAtomicCounter>::Create(size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<IndirectBuffer> IndirectBuffer::Create(const void* data, uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::Vulkan:  return Ref<VulkanIndirectBuffer>::Create(data, size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::Type::OpenGL:  return Ref<OpenGLUniformBuffer>::Create(size, binding);
		case RendererAPI::Type::Vulkan:  return Ref<VulkanUniformBuffer>::Create(size, binding);
		}

		XYZ_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
