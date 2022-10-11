#pragma once
#include "Buffer.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

namespace XYZ {
	class VertexBufferSet : public RefCount
	{
	public:
		VertexBufferSet(uint32_t frames, uint32_t size);

		void Update(const void* data, uint32_t size, uint32_t offset = 0);

		Ref<VertexBuffer> GetVertexBuffer(uint32_t frame) const { return m_VertexBuffers[frame]; }
	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		ThreadQueue<ByteBuffer>		   m_Buffers;
		uint32_t					   m_Size;
	};
}