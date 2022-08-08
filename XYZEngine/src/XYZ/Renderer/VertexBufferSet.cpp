#include "stdafx.h"
#include "VertexBufferSet.h"

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {
	VertexBufferSet::VertexBufferSet(uint32_t frames, uint32_t size)
		:
		m_Size(size)
	{
		m_VertexBuffers.resize(static_cast<size_t>(frames));
		for (auto& vertexBuffer : m_VertexBuffers)
			vertexBuffer = VertexBuffer::Create(size);
	}
	void VertexBufferSet::Update(const void* data, uint32_t size, uint32_t offset)
	{
		Ref<VertexBufferSet> instance = this;
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(data, size, offset);

		Renderer::Submit([buffer, instance, size, offset]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->m_VertexBuffers[frame]->RT_Update(buffer.Data, size, offset);
			instance->m_Buffers.PushFront(buffer);
		});
	}
}