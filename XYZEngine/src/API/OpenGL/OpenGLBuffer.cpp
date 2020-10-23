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

	void OpenGLVertexBuffer::Update(void* vertices, uint32_t size, uint32_t offset)
	{
		ByteBuffer data = ByteBuffer::Copy(vertices, size);
		Ref<OpenGLVertexBuffer> instance = this;
		Renderer::Submit([this, data, size, offset]() {		
			XYZ_ASSERT(m_Usage == BufferUsage::Dynamic, "Buffer does not have dynamic usage");
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			delete[]data;
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
		memcpy(m_LocalData, indices, count * sizeof(uint32_t));
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



	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(float* data, uint32_t size, BufferUsage usage)
		:m_Size(size), m_Usage(usage)
	{
		m_LocalData = ByteBuffer::Copy(data, size);
		Renderer::Submit([=]() {glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		switch (m_Usage)
		{
		case BufferUsage::Static:    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW); break;
		case BufferUsage::Dynamic:   glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW); break;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			});
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		delete[]m_LocalData;
		Renderer::Submit([=]() {
			glDeleteBuffers(1, &m_RendererID);
			});
	}

	void OpenGLShaderStorageBuffer::BindBase(uint32_t index)const
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID);
			});
	}


	void OpenGLShaderStorageBuffer::BindRange(uint32_t offset, uint32_t size, uint32_t index)const
	{
		Renderer::Submit([=]() {glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, m_RendererID, offset, size); });
	}

	void OpenGLShaderStorageBuffer::Bind()const
	{
		Renderer::Submit([=]() {glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); });
	}

	void OpenGLShaderStorageBuffer::Update(void* data, uint32_t size, uint32_t offset)
	{
		m_LocalData.Write(data, size, offset);
		Renderer::Submit([=]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			});
	}

	void OpenGLShaderStorageBuffer::Resize(void* data, uint32_t size)
	{
		m_LocalData.Write(data, size);
		Renderer::Submit([=]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			switch (m_Usage)
			{
			case BufferUsage::Static:    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); break;
			case BufferUsage::Dynamic:   glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); break;
			}
			});
	}
	void OpenGLShaderStorageBuffer::GetSubData(void* buffer, uint32_t size, uint32_t offset)
	{
		Renderer::Submit([=]() {
			XYZ_ASSERT(size + offset < m_Size, "Accesing data out of range");
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, buffer);
			});
	}
	OpenGLAtomicCounter::OpenGLAtomicCounter(uint32_t numOfCounters)
		: m_NumberOfCounters(numOfCounters), m_Counters(new uint32_t[numOfCounters])
	{
		Renderer::Submit([=]() {
			// Make sure it is initialized as zero
			for (size_t i = 0; i < numOfCounters; ++i)
				m_Counters[i] = 0;

			glGenBuffers(1, &m_RendererID);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t) * numOfCounters, m_Counters, GL_DYNAMIC_DRAW);
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
		Renderer::Submit([=]() {
			for (size_t i = 0; i < m_NumberOfCounters; ++i)
				m_Counters[i] = 0;
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t) * m_NumberOfCounters, m_Counters);
			});
	}
	void OpenGLAtomicCounter::BindBase(uint32_t index)const
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, m_RendererID);
			});
	}
	void OpenGLAtomicCounter::Update(uint32_t* data, uint32_t count, uint32_t offset)
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, offset * sizeof(uint32_t), sizeof(uint32_t) * m_NumberOfCounters, data);
			});
	}
	uint32_t* OpenGLAtomicCounter::GetCounters()
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
		glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t) * m_NumberOfCounters, m_Counters);

		return m_Counters;
	}
	OpenGLIndirectBuffer::OpenGLIndirectBuffer(void* drawCommand, uint32_t size)
		:
		m_Size(size)
	{
		Renderer::Submit([=]() {
			glGenBuffers(1, &m_RendererID);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID);
			glBufferData(GL_DRAW_INDIRECT_BUFFER, size, drawCommand, GL_STATIC_DRAW);
			});
	}
	OpenGLIndirectBuffer::~OpenGLIndirectBuffer()
	{
		Renderer::Submit([=]() {glDeleteBuffers(1, &m_RendererID); });
	}
	void OpenGLIndirectBuffer::Bind()const
	{
		Renderer::Submit([=]() {glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID); });
	}
	void OpenGLIndirectBuffer::BindBase(uint32_t index)
	{
		Renderer::Submit([=]() {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID);
			});
	}
}