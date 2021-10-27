#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include "XYZ/Utils/DataStructures/Queue.h"

namespace XYZ {
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const void* vertices, uint32_t size, BufferUsage usage);
		OpenGLVertexBuffer(uint32_t size);
		virtual ~OpenGLVertexBuffer() override;
		virtual void Release() const override;
		virtual void Bind() const override;
		virtual void UnBind() const override;
		virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) override;
		virtual void Resize(const void* vertices, uint32_t size) override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const BufferLayout& GetLayout() const override { return m_Layout; };
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
		BufferUsage m_Usage;
		BufferLayout m_Layout;
		Queue<ByteBuffer> m_Buffers;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer() override;
		virtual void Release() const override;
		virtual void Bind() const override;
		virtual void UnBind() const override;
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count;
		ByteBuffer m_LocalData;
	};

	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(const void* data, uint32_t size, uint32_t binding, BufferUsage usage);
		virtual ~OpenGLShaderStorageBuffer() override;
		virtual void Release() const override;
		virtual void BindBase(uint32_t binding) const override;
		virtual void BindRange(uint32_t offset, uint32_t size) const override;
		virtual void Bind()const override;
		virtual void Update(void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void Resize(void* data, uint32_t size) override;
		virtual void GetSubData(void** buffer, uint32_t size, uint32_t offset = 0) override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };;
		virtual const BufferLayout& GetLayout() const override { return m_Layout; };
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
		uint32_t m_Binding;
		BufferUsage m_Usage;
		BufferLayout m_Layout;
		Queue<ByteBuffer> m_Buffers;
	};


	class OpenGLAtomicCounter : public AtomicCounter
	{
	public:
		OpenGLAtomicCounter(uint32_t numOfCounters, uint32_t binding);
		virtual ~OpenGLAtomicCounter() override;
		virtual void Release() const override;
		virtual void Reset() override;
		virtual void BindBase(uint32_t index)const override;
		virtual void Update(uint32_t* data, uint32_t count, uint32_t offset) override;
		virtual uint32_t* GetCounters();
		virtual uint32_t GetNumCounters() { return m_NumberOfCounters; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_NumberOfCounters;
		uint32_t* m_Counters;
	};



	class OpenGLIndirectBuffer : public IndirectBuffer
	{
	public:
		OpenGLIndirectBuffer(void* drawCommand, uint32_t size, uint32_t binding);
		virtual ~OpenGLIndirectBuffer() override;
		virtual void Release() const override;
		virtual void Bind()const override;
		virtual void BindBase(uint32_t index) override;
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
	};


	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer() override;
		virtual void Release() const override;
		virtual void Update(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID = 0;
	};

}