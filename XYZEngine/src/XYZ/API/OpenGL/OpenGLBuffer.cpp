#include "stdafx.h"
#include "OpenGLBuffer.h"

#include <GL/glew.h>

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size, BufferUsage usage)
		: m_Size(size), m_Usage(usage)
	{
		m_LocalData = ByteBuffer::Copy(vertices, size);
		Renderer::Submit([=]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			switch (usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, m_LocalData, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, m_LocalData, GL_DYNAMIC_DRAW); break;
			}
		});
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
		: m_Size(size), m_Usage(BufferUsage::Dynamic)
	{
		m_LocalData.Allocate(size);
		Renderer::Submit([this]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ARRAY_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
			});
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		delete[]m_LocalData;
		Renderer::Submit([this]() {
			glDeleteBuffers(1, &m_RendererID);
		});
	}

	void OpenGLVertexBuffer::Bind() const
	{
		Renderer::Submit([this]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			});
	}

	void OpenGLVertexBuffer::UnBind() const
	{
		Renderer::Submit([]() {glBindBuffer(GL_ARRAY_BUFFER, 0); });
	}

	void OpenGLVertexBuffer::UpdateNoCopy(void* vertices, uint32_t size, uint32_t offset) const
	{
		ByteBuffer buffer((uint8_t*)vertices, size);
		Ref<const OpenGLVertexBuffer> instance = this;
		Renderer::Submit([instance, offset, size, buffer]() {
			XYZ_ASSERT(instance->m_Usage == BufferUsage::Dynamic, "Buffer does not have dynamic usage");
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
		});
	}

	void OpenGLVertexBuffer::Update(void* vertices, uint32_t size, uint32_t offset)
	{
		// TODO: use multiple buffers instead of copying
		ByteBuffer buffer = ByteBuffer::Copy(vertices, size);
		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit([instance, offset, size, buffer]() {
			XYZ_ASSERT(instance->m_Usage == BufferUsage::Dynamic, "Buffer does not have dynamic usage");
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
			delete[] buffer;
		});
	}

	void OpenGLVertexBuffer::Resize(float* vertices, uint32_t size)
	{
		Renderer::Submit([=]() {
			m_Size = size;
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			switch (m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, m_LocalData, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, m_LocalData, GL_DYNAMIC_DRAW); break;
			}
			});
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		m_LocalData = ByteBuffer::Copy(indices, count * sizeof(uint32_t));

		Renderer::Submit([this]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Count * sizeof(unsigned int), m_LocalData, GL_STATIC_DRAW);
		});
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		delete[]m_LocalData;
		Renderer::Submit([this]() {
			glDeleteBuffers(1, &m_RendererID);
			});
	}

	void OpenGLIndexBuffer::Bind() const
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			});
	}

	void OpenGLIndexBuffer::UnBind() const
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			});
	}



	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(void* data, uint32_t size, uint32_t binding, BufferUsage usage)
		:m_Size(size), m_Binding(binding), m_Usage(usage)
	{
		if (data)
			m_LocalData = ByteBuffer::Copy(data, size);
		else
			m_LocalData.Allocate(size);

		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, binding]() mutable {
			glGenBuffers(1, &instance->m_RendererID);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance->m_RendererID);
			switch (instance->m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_SHADER_STORAGE_BUFFER, size, instance->m_LocalData, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_SHADER_STORAGE_BUFFER, size, instance->m_LocalData, GL_DYNAMIC_DRAW); break;
			}
			glBindBufferBase(GL_UNIFORM_BUFFER, binding, instance->m_RendererID);
		});
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		if (m_LocalData)
			delete[]m_LocalData;
		Renderer::Submit([=]() {
			glDeleteBuffers(1, &m_RendererID);
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
		ByteBuffer buffer = ByteBuffer::Copy(data, size);
		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer] () {
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
			delete[] buffer;
		});
	}

	void OpenGLShaderStorageBuffer::Resize(void* data, uint32_t size)
	{
		ByteBuffer buffer(nullptr, size);
		if (data)
			buffer.Write(data, size, 0);

		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, size, buffer]() {
			glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
			switch (instance->m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_DYNAMIC_DRAW); break;
			}
			delete[]buffer;
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
		});
	}
	OpenGLAtomicCounter::~OpenGLAtomicCounter()
	{
		Renderer::Submit([=]() {
			glDeleteBuffers(1, &m_RendererID);
			delete[]m_Counters;
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
		});
	}
	OpenGLIndirectBuffer::~OpenGLIndirectBuffer()
	{
		Renderer::Submit([=]() {glDeleteBuffers(1, &m_RendererID); });
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
		Ref<OpenGLUniformBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glDeleteBuffers(1, &instance->m_RendererID);
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