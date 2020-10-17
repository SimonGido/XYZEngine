#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

namespace XYZ {
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* vertices, uint32_t size, BufferUsage usage);
		OpenGLVertexBuffer(uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;
		virtual void Update(void* vertices, uint32_t size, uint32_t offset = 0) override;
		virtual void Resize(float* vertices, uint32_t size) override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };

		virtual const BufferLayout& GetLayout() const override { return m_Layout; };
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
		BufferUsage m_Usage;
		BufferLayout m_Layout;
		ByteBuffer m_LocalData;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count;
		ByteBuffer m_LocalData;
	};

	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(float* data, uint32_t size, BufferUsage usage);
		virtual ~OpenGLShaderStorageBuffer();
		virtual void BindBase(uint32_t index) const override;
		virtual void BindRange(uint32_t offset, uint32_t size, uint32_t index)const override;
		virtual void Bind()const override;
		virtual void Update(void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void Resize(void* data, uint32_t size) override;
		virtual void GetSubData(void* buffer, uint32_t size, uint32_t offset = 0) override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };;
		virtual const BufferLayout& GetLayout() const override { return m_Layout; };

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
		BufferUsage m_Usage;
		BufferLayout m_Layout;
		ByteBuffer m_LocalData;
	};


	class OpenGLAtomicCounter : public AtomicCounter
	{
	public:
		OpenGLAtomicCounter(uint32_t numOfCounters);
		virtual ~OpenGLAtomicCounter();

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
		OpenGLIndirectBuffer(void* drawCommand, uint32_t size);
		virtual ~OpenGLIndirectBuffer();

		virtual void Bind()const override;
		virtual void BindBase(uint32_t index) override;
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
	};

}