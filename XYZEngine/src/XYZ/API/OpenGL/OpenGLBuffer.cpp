#include "stdafx.h"
#include "OpenGLBuffer.h"

#include <GL/glew.h>

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {
	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* vertices, uint32_t size, BufferUsage usage)
		: m_Size(size), m_Usage(usage)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		if (vertices)
			buffer.Write(vertices, size, 0);
		Renderer::Submit([=]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			switch (usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_DYNAMIC_DRAW); break;
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
		: m_Size(size), m_Usage(BufferUsage::Dynamic)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		Renderer::Submit([this, buffer]() mutable {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ARRAY_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			m_Buffers.EmplaceBack(std::move(buffer));
		});
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}	
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}

	void OpenGLVertexBuffer::Bind() const
	{
		Ref<const OpenGLVertexBuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
		});
	}

	void OpenGLVertexBuffer::UnBind() const
	{
		Renderer::Submit([]() {glBindBuffer(GL_ARRAY_BUFFER, 0); });
	}


	void OpenGLVertexBuffer::Update(const void* vertices, uint32_t size, uint32_t offset)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(vertices, size, offset);

		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit([instance, offset, size, buffer]() mutable {
			XYZ_ASSERT(instance->m_Usage == BufferUsage::Dynamic, "Buffer does not have dynamic usage");
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
			instance->m_Buffers.EmplaceBack(std::move(buffer));
		});
	}

	void OpenGLVertexBuffer::Resize(const void* vertices, uint32_t size)
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		ByteBuffer buffer;
		buffer = ByteBuffer::Copy(vertices, size);

		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit([instance, size, buffer]() mutable {
			instance->m_Size = size;
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			switch (instance->m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_DYNAMIC_DRAW); break;
			}
			instance->m_Buffers.PushBack(buffer);
		});
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		m_LocalData = ByteBuffer::Copy(indices, count * sizeof(uint32_t));
		
		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glCreateBuffers(1, &instance->m_RendererID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->m_Count * sizeof(unsigned int), instance->m_LocalData, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		});
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		delete[]m_LocalData;
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}


	void OpenGLIndexBuffer::Bind() const
	{
		Ref<const OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID);
		});
	}

	void OpenGLIndexBuffer::UnBind() const
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		});
	}



	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(const void* data, uint32_t size, uint32_t binding, BufferUsage usage)
		:m_Size(size), m_Binding(binding), m_Usage(usage)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		if (data)
			buffer.Write(data, size, 0);

		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, binding, buffer]() mutable {
			glGenBuffers(1, &instance->m_RendererID);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance->m_RendererID);
			switch (instance->m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_SHADER_STORAGE_BUFFER, size, buffer, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_SHADER_STORAGE_BUFFER, size, buffer, GL_DYNAMIC_DRAW); break;
			}
			glBindBufferBase(GL_UNIFORM_BUFFER, binding, instance->m_RendererID);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		});
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}

	void OpenGLShaderStorageBuffer::BindBase(uint32_t binding) const
	{
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, binding]() {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance->m_RendererID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, instance->m_RendererID);
		});
	}


	void OpenGLShaderStorageBuffer::BindRange(uint32_t offset, uint32_t size) const
	{
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, offset, size]() {
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, instance->m_Binding, instance->m_RendererID, offset, size);

		});
	}

	void OpenGLShaderStorageBuffer::Bind()const
	{
		Renderer::Submit([=]() {glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); });
	}

	void OpenGLShaderStorageBuffer::Update(void* data, uint32_t size, uint32_t offset)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(data, size, offset);

		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer]() mutable {
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
			instance->m_Buffers.PushBack(buffer);
		});
	}

	void OpenGLShaderStorageBuffer::Resize(void* data, uint32_t size)
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		ByteBuffer buffer;
		buffer = ByteBuffer::Copy(data, size);

		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, buffer]() mutable {
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			switch (instance->m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_DYNAMIC_DRAW); break;
			}
			instance->m_Buffers.PushBack(buffer);
		});
	}
	void OpenGLShaderStorageBuffer::GetSubData(void** buffer, uint32_t size, uint32_t offset)
	{
		XYZ_ASSERT(size + offset <= m_Size, "Accesing data out of range");
		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, buffer, size, offset]() {
			*buffer = new uint8_t[size];
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, buffer);
		});
	}
	OpenGLAtomicCounter::OpenGLAtomicCounter(uint32_t numOfCounters, uint32_t binding)
		: m_NumberOfCounters(numOfCounters), m_Counters(new uint32_t[numOfCounters])
	{
		Ref<OpenGLAtomicCounter> instance = this;
		Renderer::Submit([instance, numOfCounters, binding]() mutable {

			for (size_t i = 0; i < numOfCounters; ++i)
				instance->m_Counters[i] = 0;

			glCreateBuffers(1, &instance->m_RendererID);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, instance->m_RendererID);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t) * numOfCounters, instance->m_Counters, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding, instance->m_RendererID);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		});
	}
	OpenGLAtomicCounter::~OpenGLAtomicCounter()
	{
		uint32_t rendererID = m_RendererID;
		uint32_t* counters = m_Counters;
		Renderer::Submit([rendererID, counters]() {
			glDeleteBuffers(1, &rendererID);
			delete[]counters;
		});
	}

	void OpenGLAtomicCounter::Reset()
	{
		Ref<OpenGLAtomicCounter> instance = this;
		Renderer::Submit([instance]() mutable {
			for (size_t i = 0; i < instance->m_NumberOfCounters; ++i)
				instance->m_Counters[i] = 0;
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t) * instance->m_NumberOfCounters, instance->m_Counters);
		});
	}
	void OpenGLAtomicCounter::BindBase(uint32_t index) const
	{
		Ref<const OpenGLAtomicCounter> instance = this;
		Renderer::Submit([instance, index]() {
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, instance->m_RendererID);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, instance->m_RendererID);
		});
	}
	void OpenGLAtomicCounter::Update(uint32_t* data, uint32_t count, uint32_t offset)
	{
		Ref<OpenGLAtomicCounter> instance = this;
		ByteBuffer buffer;
		buffer.Allocate(count * sizeof(uint32_t));
		buffer.Write(data, count * sizeof(uint32_t));
		Renderer::Submit([instance, offset, count, buffer]() {
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, offset * sizeof(uint32_t), sizeof(uint32_t) * instance->m_NumberOfCounters, buffer);
			delete[]buffer;
		});
	}
	uint32_t* OpenGLAtomicCounter::GetCounters()
	{
		Ref<OpenGLAtomicCounter> instance = this;
		Renderer::Submit([instance] {
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, instance->m_RendererID);
			glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t) * instance->m_NumberOfCounters, instance->m_Counters);
		});
		return m_Counters;
	}
	OpenGLIndirectBuffer::OpenGLIndirectBuffer(void* drawCommand, uint32_t size, uint32_t binding)
		:
		m_Size(size)
	{
		Ref<OpenGLIndirectBuffer> instance = this;
		Renderer::Submit([instance, size, drawCommand, binding]() mutable {
			glGenBuffers(1, &instance->m_RendererID);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, instance->m_RendererID);
			glBufferData(GL_DRAW_INDIRECT_BUFFER, size, drawCommand, GL_STATIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, instance->m_RendererID);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		});
	}
	OpenGLIndirectBuffer::~OpenGLIndirectBuffer()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}

	void OpenGLIndirectBuffer::Bind()const
	{
		Renderer::Submit([this]() {glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID); });
	}
	void OpenGLIndirectBuffer::BindBase(uint32_t index)
	{
		Renderer::Submit([this, index]() {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID);
		});
	}


	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		Ref<OpenGLUniformBuffer> instance = this;
		Renderer::Submit([instance, size, binding]() mutable {
			glCreateBuffers(1, &instance->m_RendererID);
			glNamedBufferData(instance->m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, binding, instance->m_RendererID);
		});
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}

	void OpenGLUniformBuffer::Update(const void* data, uint32_t size, uint32_t offset)
	{
		Ref<OpenGLUniformBuffer> instance = this;
		ByteBuffer buffer;
		buffer.Allocate(size);
		buffer.Write((void*)data, size, offset);
		Renderer::Submit([instance, offset, size, buffer]() mutable {
			glNamedBufferSubData(instance->m_RendererID, offset, size, buffer);
			delete[]buffer;
		});
	}
}